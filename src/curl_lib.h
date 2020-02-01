/*****************************************************************************
 * @file    curl_lib.h
 * @brief   Source code .h file for curl_lib module
 * @date    2020-JAN-28
 *
 * @ingroup curl_lib
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020, Thomas Bresson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
*****************************************************************************/

#ifndef CURL_LIB_H
#define CURL_LIB_H

/**
 * @addtogroup curl_lib
 * @{
 */
/* ***************************    Includes     **************************** */

// Include for the error return type
#include "errors.h"
#include "inc/shared_data_types.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */


/* ***********************   Function Prototypes   ************************ */

void curlLibInit(void);
void curlLibBufferInit(httpDataBuffer_t *const p_buff);
appErrors_t curlLibGetData(httpDataBuffer_t *const p_buffer, const char *const url);
void curlLibFreeData(const httpDataBuffer_t *const p_buffer);

/** @} */

#endif /* CURL_LIB_H */
