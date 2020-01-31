//////////////////////////////////////////////////////////////////////////////
//
//  json_deserialization.h
//
//  JSON Deserialization Utilties
//
//  Deserializes JSON
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

#ifndef JSON_DESERIALIZATION_H
#define JSON_DESERIALIZATION_H

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

typedef struct
{
    int num_tokens;      // Number of tokens in the dataset
    jsmntok_t *p_tokens; // Pointer to the tokens
} jsmnTokenizationData_t;

/* ***********************   Function Prototypes   ************************ */

int jsonSearchForElement(const jsmnTokenizationData_t *const p_tok_data, const char *const p_json_buff,
                         const jsonKeyValue_t *const p_key_val);
void jsonDeserializeElement(const jsonKeyValue_t *const p_key_value,
                                   const jsmntok_t *const p_value_tok,
                                   const char *const p_js_buffer, void *const p_data);

#endif /* JSON_DESERIALIZATION_H */
