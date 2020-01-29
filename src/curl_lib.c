/*****************************************************************************
 * @file    curl_lib.c
 * @brief   Source code .c file for curl_lib module
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

/**
 * @addtogroup curl_lib
 * @{
 */

/* ***************************    Includes     **************************** */

// Std
#include <stdio.h>
#include <stdint.h>

// Libs
#include "inc/curl/curl.h"

//Module
#include "errors.h"
#include "curl_lib.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static size_t curlLibStoreJsonData(void *ptr, size_t size, size_t nmemb, void *stream);

/* ***********************   File Scope Variables   *********************** */

/* ****************************   BEGIN CODE   **************************** */

/* *************************   Public  Functions   ************************ */

// Init curl and static members
void curlLibInit(void)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void curlLibBufferInit(httpDataBuffer_t *const p_buff)
{
    memcpy(p_buff, 0, sizeof(httpDataBuffer_t));
}

appErrors_t curlLibGetData(httpDataBuffer_t *const p_buffer, const char *const url)
{
    CURL *curl_handle = curl_easy_init();

    if (curl_handle)
    {
        CURLcode res;
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        res = curl_easy_perform(curl_handle);
        if (res == CURLE_OK)
        {
            // Check the size
            // TODO: This should be fixed in a way that:
            // TODO: a) Doesn't require downloading the payload twice
            // TODO: b) Isn't vulnerable to the payload changing in size between the 1st and 2nd download, wrecking the allocation
            curl_off_t dl_size;
            res = curl_easy_getinfo(curl_handle, CURLINFO_SIZE_DOWNLOAD_T, &dl_size);
            if (res == CURLE_OK)
            {
                // Check to see if size buffer is large enough to hold the JSON data
                if (p_buffer->size < dl_size)
                {
                    // Free existing and malloc a larger buffer size before download
                    if (p_buffer->p_buffer != NULL)
                    {
                        // TODO: Change this free to a `realloc` call
                        free(p_buffer->p_buffer);
                        p_buffer->size = 0;
                    }

                    // Malloc a new buffer
                    p_buffer->p_buffer = malloc((size_t)dl_size);

                    if (p_buffer->p_buffer != NULL)
                    {
                        // Set position to start and size to the malloc'd size
                        p_buffer->p_pos = p_buffer->p_buffer;
                        p_buffer->size = dl_size;
                    }
                    else
                    {
                        // malloc failed
                    }
                }

                if (p_buffer->size >= dl_size)
                {
                    // Setup the transfer, passing the buffer struct into the callback
                    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, p_buffer);
                    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlLibStoreJsonData);
                    // Kick off the download with the write function in place to store the data
                    res = curl_easy_perform(curl_handle);

                    // Set the content length of the buffer only if the download was successful
                    p_buffer->content_length = (res == CURLE_OK) ? dl_size : 0;
                }
                else
                {
                    // Something bad happened. It's possible the malloc failed
                }
            }
        }

        // Clean-up after operation complete
        curl_easy_cleanup(curl_handle);
    }

    // TODO: Make a meaningful return value
}

// Frees the buffer object after it has served it's purpose
void curlLibFreeData(const httpDataBuffer_t *const p_buffer)
{
    free(p_buffer->p_buffer);
}

/* *************************   Private Functions   ************************ */

// Write callback for libcUrl
// Writes to a buffer struct handed though the userdata pointer
static size_t curlLibStoreJsonData(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    httpDataBuffer_t *p_buffer = (httpDataBuffer_t *)userdata;
    const size_t spare_bytes = (p_buffer->size - (p_buffer->p_pos - p_buffer->p_buffer));
    const size_t num_bytes_to_copy = ((nmemb <= spare_bytes) ? nmemb : spare_bytes);
    memcpy(p_buffer->p_pos, ptr, num_bytes_to_copy);

    // Advance the buffer position
    p_buffer->p_pos += num_bytes_to_copy;

    return num_bytes_to_copy;
}
