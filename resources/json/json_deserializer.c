//////////////////////////////////////////////////////////////////////////////
//
//  json_deserializer.c
//
//  JSON Deserializer
//
//  This module will deserialize JSON string data into C data types based
//  on a table configuration and list of JSMN tokens.
//
//  NOTE: The JSON parser used is operating in non-strict mode.
//  See https://zserge.com/jsmn.html for details. ext below pulled from this link.
//
//  In non-strict mode jsmn accepts:
//  * non-quoted primitive values other than true/false/null/numbers
//  * Only primitive values without a root object
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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

// Project Includes
#include "enum_label.h"
#include "jsmn/jsmn.h"

// Module Includes
#include "json_deserializer.h"

/* ***************************   Definitions   **************************** */

#define MODULE_NAME_DEBUG       "JSON_Desr"

/* ****************************   Structures   **************************** */

// Struct to be used when traversing the tree.
// Will be pushed and popped off a stack so as they are determined, they can
// be parsed out later.
typedef struct
{
    const jsonContainer_t *p_object;
    int start_tok_idx;
    int end_tok_idx;
} jsonObjectData_t;

/* ***********************   Function Prototypes   ************************ */

static bool jsonFindObjectBounds(jsonObjectData_t *const p_object_data,
                                 const jsonContainer_t *p_object,
                                 const jsmntok_t *const p_token_list, const int num_tokens,
                                 const int start_tok_idx, const int end_tok_idx,
                                 const char *const p_js_buffer);
static int jsonTraverseObjectForKeys(const jsmntok_t *const p_start_tok, const int num_tokens,
                                     const jsonKeyValueList_t *const p_keylist,
                                     const char *const p_js_buffer, void *const p_data, bool *const p_element_found);

static int jsonFindKey(const jsonKeyValueList_t *const p_keylist,
                       const jsmntok_t *const p_tok, const char *const p_js_buffer);
static bool jsonTokenValid(const jsmntok_t *const p_tok);
static bool jsonTokenStrEq(const char *const p_js_buffer, const jsmntok_t *const p_token,
                           const char *const p_str);

static void jsonDeserializeElement(const jsonKeyValue_t *const p_key_value,
                                   const jsmntok_t *const p_value_tok,
                                   const char *const p_js_buffer, void *const p_data);
static void jsonDeserializePrimitive(const char *const p_token_str, const int token_len,
                                     void *const p_dest, const int dest_size, const jsonCPrimitiveType_t c_type);
static void jsonDeserializeString(const char *const p_token_str, const int token_len,
                                  char *const p_dest, const int dest_len);
static void jsonDeserializeEnum(const char *const p_token_str, const int token_len,
                                void *const p_dest, const int dest_size,
                                const enumLabel_t *const p_enum_labels,
                                const char *const p_key_str);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

// Initialize the JSON Deserialzer Module
void jsonDeserializeInit(void)
{
    // Don't call more than once!
    static bool initialized = false;
    assert(!initialized);
    initialized = true;
}


int jsonDeserialize(const jsmntok_t *const p_jsmn_tok, const int num_tokens,
                    const jsonContainer_t *const p_root_object,
                    const char *const p_js_buffer)
{
    assert((p_jsmn_tok != NULL) && (p_root_object != NULL) && (p_js_buffer != NULL) &&
           (p_root_object->p_data != NULL));

    // Root must ALWAYS be an object
    assert(p_root_object->type == E_JSON_OBJECT);

    // Track the total number of key-values found/populated
    int num_key_values_found = 0;

    // Validate the token list starts with an outer object.
    // Arrays of data, at the outer most level, are not currently supported.
    // TODO: Would be useful to distinguish logged errors for these two particular conditions.
    if ((num_tokens > 0) && (p_jsmn_tok->type == JSMN_OBJECT))
    {
        const jsonContainer_t *p_curr_object = p_root_object;
        // Set object data up for root object, this way there's no
        // special case in the loop below
        jsonObjectData_t curr_object_data = {0};
        curr_object_data.p_object = NULL;
        curr_object_data.start_tok_idx = 0;
        curr_object_data.end_tok_idx = (num_tokens - 1);

        do
        {
            // TODO: Need to implement tree traversal functionality to support
            // TODO: objects with more than one child

            // TODO: Could be done iteratively by pushing nodes onto a stack, as they are
            // TODO: Visit the node. Push all of it's children nodes to a stack.
            // TODO: Process the current node. Pop top node of the stack and make it the
            // TODO: current node. (Repeat above steps until stack is empty).
            assert(p_curr_object->num_children < 2);

            // TODO: Currently all containers must be OBJECT type for deserialization
            // TODO: Support for E_JSON_ARRAY types of containers could be added
            assert(p_curr_object->type == E_JSON_OBJECT);

            // Find the start and end token for an object
            bool obj_found = jsonFindObjectBounds(&curr_object_data, p_curr_object, p_jsmn_tok, num_tokens,
                                                  curr_object_data.start_tok_idx, curr_object_data.end_tok_idx,
                                                  p_js_buffer);

            if (obj_found && (p_curr_object->p_key_list != NULL))
            {
                // Traverse the objects keylist using the start and end indicies for the
                // object as determined by the object bounds function
                int num_tok_in_obj = (curr_object_data.end_tok_idx - curr_object_data.start_tok_idx);
                int start_tok_idx = curr_object_data.start_tok_idx;
                num_key_values_found +=
                    jsonTraverseObjectForKeys(&p_jsmn_tok[start_tok_idx], num_tok_in_obj,
                                              p_curr_object->p_key_list, p_js_buffer,
                                              p_curr_object->p_data,
                                              p_curr_object->p_deserialized_elements);
            }

            // Move to the next object
            // NOTE: This currently ignores the fact there may be more than one child
            if (p_curr_object->children != NULL)
            {
                // Only dereference if children is NOT NULL
                p_curr_object = (const jsonContainer_t *)((p_curr_object->children)[0]);
            }
            else
            {
                // Set to null to exit the loop
                p_curr_object = NULL;
            }
        } while (p_curr_object != NULL); // Continue if the now current object is not NULL
    }
    else
    {
        // TODO: Log a failure
    }

    return (num_key_values_found);
}

//
// Tokenizes the JSON data from the HTTP request payload using JSMN
//
int jsonDeserializeTokenize(const char *const str_to_tokenize, size_t str_to_tokenize_len,
                            jsmntok_t *const tokens, unsigned int max_num_tokens)
{
    jsmn_parser jsmn_parser_obj;
    jsmn_init(&jsmn_parser_obj);

    // Cast the payload data as unsigned char can be treated like a signed char in this case.
    int num_tokens = jsmn_parse(&jsmn_parser_obj, str_to_tokenize, str_to_tokenize_len,
                                tokens, max_num_tokens);

    // If the returned value is non-negative, the parsing has succeeeded
    if (num_tokens < 0)
    {
        switch (num_tokens)
        {
            case JSMN_ERROR_NOMEM:
                JSON_DEBUG_MSG("%s: Not enough tokens were provided.",
                               pcTaskGetName(NULL));
                break;

            case JSMN_ERROR_INVAL:
                JSON_DEBUG_MSG("%s: Invalid character inside JSON string.",
                               pcTaskGetName(NULL));
                break;

            case JSMN_ERROR_PART:
                /* The string is not a full JSON packet, more bytes expected */
                JSON_DEBUG_MSG("%s: The string is not a full JSON packet, more bytes expected.",
                               pcTaskGetName(NULL));
                break;

            default:
                // Either new/unhandled error
                break;
        }
    }

    return num_tokens;
}

// p_object_data[out] - Data that is populated after successfully finding the object
// p_object[in]- Object to find in the token list
// p_token_list[in] - List of JSMN tokens
// num_tokens - Num tokens left in the list
// start_tok_idx and end_tok_idx - Indicates that the list to be searched is a subset of the
// whole token list. This is used when searching for objects inside objects
// p_js_buffer[in] - Buffer containing raw JSON data
static bool jsonFindObjectBounds(jsonObjectData_t *const p_object_data,
                                 const jsonContainer_t *p_object,
                                 const jsmntok_t *const p_token_list, const int num_tokens,
                                 const int start_tok_idx, const int end_tok_idx,
                                 const char *const p_js_buffer)
{
    bool object_found_and_populated = false;

    // It should be assumed that objects with strings pointers that are NULL or Empty Strings
    // are to be the root node.
    if ((p_object->key_str == NULL) || (p_object->key_str == ""))
    {
        // Only root node has a parent of (-1)
        // Test to see if inputs indicate that we should be examining the root node
        assert(start_tok_idx >= 0);
        assert(p_token_list[start_tok_idx].parent == -1);

        // Assign the whole range of tokens to the object data
        // Actual data starts at index 1, index 0 just contains the OBJECT token
        p_object_data->start_tok_idx = 1;
        p_object_data->end_tok_idx = num_tokens - 1;

        object_found_and_populated = true;
    }
    // Object is not the root object
    else
    {
        // Ensure searching the token list will not end out of bounds
        assert(end_tok_idx < num_tokens);
        // Object is not the root node, need to find the next STRING in the
        // list and check the listed parent token to see if it is an OBJECT
        int idx;
        bool object_found = false;

        for (idx = start_tok_idx; (idx <= end_tok_idx) && (!object_found); idx++)
        {
            // Iterate through the list looking for object tokens
            // Named objects should never be be the first token in the token list
            // As the order is always "JSMN_STRING" token followed by a "JSMN_OBJECT" token
            if ((p_token_list[idx].type == JSMN_OBJECT) && (idx > 0))
            {
                // Examine the previous token to see if it's string, if so, check for a match'
                int obj_str_token_idx = idx - 1;

                if (p_token_list[obj_str_token_idx].type == JSMN_STRING)
                {
                    object_found =
                        jsonTokenStrEq(p_js_buffer, &p_token_list[obj_str_token_idx],
                                       p_object->key_str);
                }
            }
        }

        if (object_found)
        {
            // Store the location of the object token, as it's name matches the desired one
            // It's at the index previous to idx's current value
            p_object_data->start_tok_idx = (idx - 1);

            // Search forward through the list of tokens, starting at the next token in the list,
            // searching for the first token who's start index (in the data string) is > than
            // the object's end param or until the end of the list is reached.
            while ((!object_found_and_populated) && (idx < num_tokens))
            {
                // If a token in the list has a greater data start index than the object token's
                // end index, the previous index is the end of the data that is in the object
                if (p_token_list[idx].start > p_token_list[p_object_data->start_tok_idx].end)
                {
                    p_object_data->end_tok_idx = (idx - 1);
                    object_found_and_populated = true;
                }
                // Alternatively, if we reach the end of the list, that ending token is the last
                // token is the ending token
                else if (idx == (num_tokens - 1))
                {
                    p_object_data->end_tok_idx = idx;
                    object_found_and_populated = true;
                }
                else
                {
                    // Do nothing
                }

                // Move to next token
                idx++;
            }
        }
    }

    return object_found_and_populated;
}

// p_start_tok Pointer to the start token
// num_tokens Number of tokens to examine
// p_keylist List of keys
// p_js_buffer Buffer containing JSON data
// p_data Pointer to data element to deserialize into
// p_element_found Element Found array
static int jsonTraverseObjectForKeys(const jsmntok_t *const p_start_tok, const int num_tokens,
                                     const jsonKeyValueList_t *const p_keylist,
                                     const char *const p_js_buffer, void *const p_data, bool *const p_element_found)
{
    // Search list of tokens for string tokens
    int tok_idx = 0;
    int num_key_values_found = 0;

    // Clear the changed elements array to remove any existing values it may have in it
    if (p_element_found != NULL)
    {
        memset(p_element_found, 0, (sizeof(bool) * p_keylist->size));
    }

    while (jsonTokenValid(&p_start_tok[tok_idx]) && (tok_idx < num_tokens))
    {
        // Key values will always be string type token
        if (p_start_tok[tok_idx].type == JSMN_STRING)
        {
            // When string token found, match against list of keys
            int key_idx = jsonFindKey(p_keylist, &p_start_tok[tok_idx], p_js_buffer);

            // If the key index is not -1, the token was successfully matched against the key list
            if (key_idx != -1)
            {
                // Key was found, check next token for type match against specified value type
                if (jsonTokenValid(&p_start_tok[++tok_idx]))
                {
                    // If next token type matches, route to the proper deserializing function
                    if (p_start_tok[tok_idx].type == p_keylist->p_keys[key_idx].value_tok_type)
                    {
                        JSON_TRACE_MSG("JSON Key-Value Found. '%s'", p_keylist->p_keys[key_idx].key_str);

                        // Call the deserializer function in the deserializer struct passed into
                        // the function.
                        jsonDeserializeElement(&(p_keylist->p_keys[key_idx]),
                                               &p_start_tok[tok_idx], p_js_buffer, p_data);

                        // Increment the count of the tokens successfully matched and populated
                        num_key_values_found++;

                        // Mark the index of the changed element as true, as the value has been written
                        if (p_element_found != NULL)
                        {
                            p_element_found[key_idx] = true;
                        }
                    }
                    else
                    {
                        //! ERROR: Key string was matched but next token's type was incorrect
                        //! ERROR: Check the `key_idx` value to ensure the `c_type` for that
                        // !ERROR: key correctly matches the JSON data
                        assert(false);
                    }
                }
                else
                {
                    //! ERROR: Key was found, but next token was INVALID. Value not processed.
                    assert(false);
                }
            }
        }

        // Advance the index to look at the next token
        tok_idx++;
    }

    return num_key_values_found;
}

// Routes a token to be deserialized into a destination (p_data) based on the data in the key-value
// table and the information in the passed token (p_value_tok)
static void jsonDeserializeElement(const jsonKeyValue_t *const p_key_value,
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
    strncpy(p_dest, p_token_str, min(token_len, dest_len));

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


// TODO: Possibly add function to deserialize arrays. Should be able to be done by simply identifying
// TODO: the array and iterating through it using the primitive deserializer function


// Determines if a given key (as defined by the token and the data buffer) is contained in the
// list of keys.
// p_keys: Pointer to the list of keys
// num_keys: Number of keys in the list
// p_tok: Token representing the key to match to the list
// p_js_buffer: JSON data buffer (used in conjunction with the token to get the key)
static int jsonFindKey(const jsonKeyValueList_t *const p_keylist,
                       const jsmntok_t *const p_tok, const char *const p_js_buffer)
{
    int idx = 0;

    // Iterate through the list of keys attempting to find a match either until the
    // end of the list or until a match is found.
    while ( (idx < p_keylist->size) &&
            !(jsonTokenStrEq(p_js_buffer, p_tok, p_keylist->p_keys[idx].key_str)))
    {
        idx++;
    }

    // If idx is equal to the number of keys, key was NOT found in the list.
    // Return -1 to indicate this
    if (idx == p_keylist->size)
    {
        idx = -1;
    }

    return idx;
}

// Check the token for validity
// Each token should have a type other than JSMN_UNDEFINED, and should index into a position
// into the JSON string data (not -1).
static bool jsonTokenValid(const jsmntok_t *const p_tok)
{
    return ((p_tok->type != JSMN_UNDEFINED) && (p_tok->start != -1) && (p_tok->end != -1));
}

// Tests a token for string equalivalance against a passed in string (p_str)
static bool jsonTokenStrEq(const char *const p_js_buffer, const jsmntok_t *const p_token,
                           const char *const p_str)
{
    return (strncmp(p_js_buffer + p_token->start, p_str, p_token->end - p_token->start) == 0
            && strlen(p_str) == (size_t) (p_token->end - p_token->start));
}
