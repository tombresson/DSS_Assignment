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

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

typedef struct
{
    size_t size;
    char *p_buffer;
    char *p_pos;
} jsonDataBuffer_t;

/* ***********************   Function Prototypes   ************************ */

void curlLibInit(void);
appErrors_t curlLibGetData(jsonDataBuffer_t *const p_buffer, const char *const json_url);

/** @} */

#endif /* CURL_LIB_H */
