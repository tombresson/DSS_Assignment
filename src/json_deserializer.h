//////////////////////////////////////////////////////////////////////////////
//
//  json_deserializer.h
//
//  JSON Deserializer
//
//  This module will deserialize JSON string data into C data types based
//  on a table configuration and list of JSMN tokens.
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

#ifndef JSON_DESERIALIZER_H
#define JSON_DESERIALIZER_H

/* ***************************    Includes     **************************** */

#include "json_serialize_deserialize_types.h"

/* ***************************   Definitions   **************************** */

// Size in bytes of the temporary buffer used to do enum deserialization
#define JSON_ENUM_STR_BUFF_SIZE         40

// Debug related macros, set to 1 to enable
#define JSON_DEBUG_MSG_ENABLED        (1)
#define JSON_TRACE_MSG_ENABLED        (0)

#define JSON_DEBUG_MSG(...)
#define JSON_TRACE_MSG(...)

#if (JSON_DEBUG_MSG_ENABLED == 1)
#define JSON_MODULE_STR           MODULE_NAME_DEBUG ": "
// Prepend the module name to the debug messages
#undef JSON_DEBUG_MSG
#define JSON_DEBUG_MSG(...)       (printf(JSON_MODULE_STR __VA_ARGS__))
#if (JSON_TRACE_MSG_ENABLED == 1)
#undef JSON_TRACE_MSG
#define JSON_TRACE_MSG(...)       (printf(JSON_MODULE_STR __VA_ARGS__))
#endif
#endif

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

void jsonDeserializeInit(void);

// Parses the tokens into key-value pairs and passes these off to be populated into a datastructure
//
// p_jsmn_tok[in]: Pointer to the first token in the list, size is indicated by num_tokens
// p_js_buffer[in]: Buffer that contains the JSON payload from which the token list was generated
// p_root_object->p_data[out]: The destination struct for the deserialized data
//
// return:The number of key-values found
//
// WARN: Cannot handle root array-based JSON document
// WARN: Does NOT handle deserialization into arrays
int jsonDeserialize(const jsmntok_t *const p_jsmn_tok, const int num_tokens,
                    const jsonContainer_t *const p_root_object,
                    const char *const p_js_buffer);
int jsonDeserializeTokenize(const char *const str_to_tokenize, size_t str_to_tokenize_len,
                            jsmntok_t *const tokens, unsigned int max_num_tokens);

#endif /* JSON_DESERIALIZER_H */
