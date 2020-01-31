//////////////////////////////////////////////////////////////////////////////
//
//  json_deserialization.c
//
//  JSON Deserialization Utility
//
//  Module description in json_deserialization.h
//
// The MIT License (MIT)
//
// Copyright (c) 2020, Thomas Bresson
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////


/* ***************************    Includes     **************************** */

// Standard Includes
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// FreeRTOS Includes

// Library Includes

// Project Includes

// Module Includes
#include "json_deserialization.h"

/* ***************************   Definitions   **************************** */

#define MODULE_NAME_DEBUG       "JSON_Desr"

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static int jsonFindKeyValueToken(const jsmnTokenizationData_t *const p_tok_data, const char *const p_json_buff,
                                 const char *const key_str, const int key_str_len, const jsmntype_t type, const int parent_idx);
static void jsonDeserializeEnum(const char *const p_token_str, const int token_len,
                                void *const p_dest, const int dest_size,
                                const enumLabel_t *const p_enum_labels,
                                const char *const p_key_str);
static void jsonDeserializeString(const char *const p_token_str, const int token_len,
                                  char *const p_dest, const int dest_len);
static void jsonDeserializePrimitive(const char *const p_token_str, const int token_len,
                                     void *const p_dest, const int dest_size, const jsonCPrimitiveType_t c_type);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

// Searches for an expected object (based on a dot notation of the reference) in the json
// data and returns a JSMN token index of that data
// NOTE: There is no support of wildcards ('*' or '?') in the search
// NOTE: There is no support for indexing into arrays
int jsonSearchForElement(const jsmnTokenizationData_t *const p_tok_data, const char *const p_json_buff, const jsonKeyValue_t *const p_key_val)
{
    // Check for malformed reference
    const char *const json_ref = p_key_val->key_str;
    const jsmntype_t type = p_key_val->value_tok_type;
    assert((*json_ref != '\0') && (*json_ref != '.'));

    int current_char_idx = 0;
    int last_element_start = 0;
    // Any objects in the root start with a parent of 0
    int current_parent = 0;
    bool element_not_found = false;
    while ((json_ref[current_char_idx] != '\0') && !element_not_found)
    {
        // Look to see if current character is a '.'
        if (json_ref[current_char_idx] == '.')
        {
            // Found the end of the current reference
            // Form a string, and search for a token with the required parent
            int idx_of_value = jsonFindKeyValueToken(p_tok_data, p_json_buff, (json_ref + last_element_start), (current_char_idx - last_element_start), JSMN_OBJECT, current_parent);
            if (idx_of_value != -1)
            {
                current_parent = idx_of_value;
                // Save off an index one after the '.' character
                last_element_start = (current_char_idx + 1);
            }
            else
            {
                element_not_found = true;
            }
        }

         // Assume is part of the next reference and keep going
        current_char_idx++;
    }

    int final_value_idx = -1;
    if (!element_not_found)
    {
        // The last search is for the final element in the string
        final_value_idx = jsonFindKeyValueToken(p_tok_data, p_json_buff, (json_ref + last_element_start), (current_char_idx - last_element_start), type, current_parent);
    }

    return final_value_idx;
}

// Routes a token to be deserialized into a destination (p_data) based on the data in the key-value
// table and the information in the passed token (p_value_tok)
void jsonDeserializeElement(const jsonKeyValue_t *const p_key_value,
                                   const jsmntok_t *const p_value_tok,
                                   const char *const p_js_buffer, void *const p_data)
{
    int token_len = (p_value_tok->end - p_value_tok->start);
    const char *p_token_str = (p_js_buffer + p_value_tok->start);

    // Member size should not be zero.
    assert(p_key_value->struct_member_size != 0);

    switch (p_key_value->value_tok_type)
    {
        case JSMN_STRING:

            // Check to see if string is supposed to represent an enum
            if (p_key_value->c_type == E_JSON_C_ENUM)
            {
                jsonDeserializeEnum(p_token_str,
                                    token_len,
                                    (((uint8_t *)p_data) + p_key_value->struct_member_offset),
                                    p_key_value->struct_member_size,
                                    p_key_value->enum_labels,
                                    p_key_value->key_str);
            }
            else if (p_key_value->c_type == E_JSON_C_STR_PTR)
            {
                // Deserialize right here to the string pointer datatype
                jsonStr_t *str = (jsonStr_t *)(((uint8_t *)p_data) + p_key_value->struct_member_offset);
                str->str = p_token_str;
                str->len = token_len;
            }
            else
            {
                jsonDeserializeString(p_token_str,
                                      token_len,
                                      (char *)(((uint8_t *)p_data) + p_key_value->struct_member_offset),
                                      p_key_value->struct_member_size);
            }

            break;

        case JSMN_PRIMITIVE:
            jsonDeserializePrimitive(p_token_str,
                                     token_len,
                                     (((uint8_t *)p_data) + p_key_value->struct_member_offset),
                                     p_key_value->struct_member_size,
                                     p_key_value->c_type);
            break;

        default:
            // Unsupported deserialization of token type
            assert(false);
            break;
    }
}

/* *************************   Private Functions   ************************ */



// Finds a value's token index in the list, using the known parent and the value's key string
static int jsonFindKeyValueToken(const jsmnTokenizationData_t *const p_tok_data, const char *const p_json_buff,
                                 const char *const key_str, const int key_str_len, const jsmntype_t type, const int parent_idx)
{
    int idx = 0;
    bool key_found = false;
    while (idx < p_tok_data->num_tokens && !key_found)
    {
        // Parent object matches and the data is a string
        if (p_tok_data->p_tokens[idx].parent == parent_idx &&
            p_tok_data->p_tokens[idx].type == JSMN_STRING)
        {
            const jsmntok_t *const p_key_tok = &p_tok_data->p_tokens[idx];
            // Check to see if key string matches to the JSON string data
            // TODO: There should be a check to ensure that strncmp doesn't read outside the json buffer
            if (strncmp(key_str, &p_json_buff[p_key_tok->start], key_str_len) == 0)
            {
                // check to see the next token type matches the required type, if so, value is found
                // otherwise keep going
                key_found = (p_tok_data->p_tokens[idx + 1].type == type);
            }
        }
        ++idx;
    }

    return (key_found ? idx : -1);
}



// Get the value of a string that represents an enumerated value.
// !WARNING: A value of '0' will be written to the enum if it is not matched.
// If '0' is a VALID ENUM (some enumerationg other than NULL/UNKNOWN/etc.), it may be confusing
// to determine if the value that was deserialized is ACTUALLY the 0th enum or if it simply didn't
// match an entry in the enum label table.
static void jsonDeserializeEnum(const char *const p_token_str, const int token_len,
                                void *const p_dest, const int dest_size,
                                const enumLabel_t *const p_enum_labels,
                                const char *const p_key_str)
{
    // For deserialization, the string data must fit in to the temporary buffer
    // If this fails, increase the enum buffer size or reduce the size of the string
    // that represents the enum value
    assert(token_len < JSON_ENUM_STR_BUFF_SIZE - 1);
    assert(p_enum_labels != NULL);
    // Buffer must be initialized to 0 for string ops
    char buff[JSON_ENUM_STR_BUFF_SIZE] = {0};
    // Only every copy at a max the size of the buffer -1 (to keep the null term byte)
    memcpy(buff, p_token_str, min(token_len, (JSON_ENUM_STR_BUFF_SIZE - 1)));

    int value = 0;
    bool enum_rslt = elGetValueFromString(p_enum_labels, buff, &value, false);

    if (!enum_rslt)
    {
        value = 0;
        // This will occur if the enum that is received does not match any of the data in the
        // list. It COULD be acceptable if a single JSON element is "shared" across two separate
        // sections. Then no one enum list will contain all of the possible enumerations.
        JSON_TRACE_MSG("Unexpected value %s for enum %s. This may be acceptable.",
                       buff, p_key_str);
    }

    // Write value based on destination size
    switch (dest_size)
    {
        case 1:
            *((uint8_t *)p_dest) = (uint8_t)value;
            break;

        case 2:
            *((uint16_t *)p_dest) = (uint16_t)value;
            break;

        case 4:
            *((uint32_t *)p_dest) = (uint32_t)value;
            break;

        default:
            // Size of enum destination not supported
            assert(false);
            break;
    }
}

// Deserializes a JSON String into a C String buffer
//
// Takes a string (p_token_str) and length and writes the character data to the destination.
// Destination will be null terminated based on null_term_dest.
static void jsonDeserializeString(const char *const p_token_str, const int token_len,
                                  char *const p_dest, const int dest_len)
{
    strncpy_s(p_dest,dest_len, p_token_str, token_len);

    if (token_len > dest_len)
    {
        // String was copied but truncated.
        JSON_DEBUG_MSG("JSON document string value %.*s is %d bytes longer than the destination length of %d",
                       token_len, p_token_str, (token_len - dest_len), dest_len);
    }

    // All parsed strings will be NULL terminated.
    // Ensure either the position at end of the copied string or
    // at the end of the buffer is a null character
    p_dest[min(token_len, (dest_len - 1))] = '\0';
}

// Deserializes a JSON Primitive into a C Primitive data element (p_dest)
static void jsonDeserializePrimitive(const char *const p_token_str, const int token_len,
                                     void *const p_dest, const int dest_size, const jsonCPrimitiveType_t c_type)
{
    // Check the destination size (in bytes) is correct based on the specified expected primitive type
    switch (c_type)
    {
        case E_JSON_C_BOOL:
            assert(dest_size == (int)sizeof(bool));
            break;

        case E_JSON_C_INT8:
        case E_JSON_C_UINT8:
            assert(dest_size == 1);
            break;

        case E_JSON_C_INT16:
        case E_JSON_C_UINT16:
            assert(dest_size == 2);
            break;

        case E_JSON_C_INT32:
        case E_JSON_C_UINT32:
            assert(dest_size == 4);
            break;

        case E_JSON_C_INT64:
            assert(dest_size == (int)sizeof(int64_t));
            break;

        case E_JSON_C_FLOAT:
            assert(dest_size == (int)sizeof(float));
            break;

        default:
            // Unsupported type
            assert(false);
            break;
    }

    // Casting the token pointer from a const char to a regular char pointer
    // to avoid warnings when using the stdlib function calls with a const char pointer
    char *p_token_str_end = (char *)(p_token_str + token_len);

    // Examine the first character to determine which function handles the deserialization
    switch (tolower(p_token_str[0]))
    {
        // True
        case 't':
            *((bool *)p_dest) = true;
            break;

        // False
        case 'f':
            *((bool *)p_dest) = false;
            break;

        // Number
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

            if (c_type == E_JSON_C_INT8)
            {
                // It is possible this will overflow, but it will be safe
                *((int8_t *)p_dest) = (int8_t)strtol(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_INT16)
            {
                // It is possible this will overflow, but it will be safe
                *((int16_t *)p_dest) = (int16_t)strtol(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_INT32)
            {
                *((int32_t *)p_dest) = strtol(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_INT64)
            {
                *((int64_t *)p_dest) = strtoll(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_UINT8)
            {
                // It is possible this will overflow, but it will be safe
                *((uint8_t *)p_dest) = (uint8_t)strtoul(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_UINT16)
            {
                // It is possible this will overflow, but it will be safe
                *((uint16_t *)p_dest) = (uint16_t)strtoul(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_UINT32)
            {
                *((uint32_t *)p_dest) = strtoul(p_token_str, &p_token_str_end, 10);
            }
            else if (c_type == E_JSON_C_FLOAT)
            {
                *((float *)p_dest) = strtof(p_token_str, &p_token_str_end);
            }
            else
            {
                // No other conversion types currently supported.
                assert(false);
            }

            break;

        // Null
        case 'n':

            if (c_type == E_JSON_C_BOOL)
            {
                *((bool *)p_dest) = 0;
            }
            else if ((c_type == E_JSON_C_INT8) || (c_type == E_JSON_C_UINT8))
            {
                *((uint8_t *)p_dest) = 0;
            }
            else if ((c_type == E_JSON_C_INT32) || (c_type == E_JSON_C_UINT32))
            {
                *((uint32_t *)p_dest) = 0;
            }
            else if (c_type == E_JSON_C_INT64)
            {
                *((int64_t *)p_dest) = 0;
            }
            else if (c_type == E_JSON_C_FLOAT)
            {
                *((float *)p_dest) = 0.0;
            }
            else
            {
                // No other conversion types currently supported.
                assert(false);
            }

            break;

        default:
            // Primitive not recognized. This should never happen.
            assert(false);
    }
}
