//////////////////////////////////////////////////////////////////////////////
//
//  json_data_buffer.c
//
//  JSON Data Buffer
//
//  Module allows other modules to allocate and free buffers, primarily for
//  the sake of buffering JSON data to be transmitted.
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
#include <assert.h>
#include <string.h>

// Library Includes

// Project Includes

// Module Includes
#include "json_data_buffer.h"

/* ***************************   Definitions   **************************** */

const jsonDataBuffer_t DEFAULT_BUFF = {.p_buff = NULL, .size = 0, .id = 0};

/* ****************************   Structures   **************************** */

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

//
// Initialize this module
//
void jsonBufferInit(void)
{

}

// Attempt to take a buffer from the pool
// Function will wait for a buffer to be available until the given timeout
bool jsonBufferGetBuffer(jsonDataBuffer_t *const p_buffer, const int required_size)
{

}

// Returns a buffer to the pool
void jsonBufferReturnBuffer(jsonDataBuffer_t *const p_buff)
{

}
