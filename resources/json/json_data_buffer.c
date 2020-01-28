//////////////////////////////////////////////////////////////////////////////
//
//  json_data_buffer.c
//
//  JSON Data Buffer
//
//  Module allows other modules to allocate and free buffers, primarily for
//  the sake of buffering JSON data to be transmitted.
//
//  Copyright Fortune Brands Home & Security, 2018
//  All Rights Reserved.  This is a confidential intellectual property work
//  of Fortune Brands Home & Security and is not to be copied or transmitted to
//  other parties without the express written permission of Fortune Brands
//  Home & Security.
//
//////////////////////////////////////////////////////////////////////////////

/* ***************************    Includes     **************************** */

// Standard Includes
#include <stdbool.h>
#include <assert.h>
#include <string.h>

// FreeRTOS Includes
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

// Library Includes
#include "aws_bufferpool_config.h"

// Project Includes

// Module Includes
#include "json_data_buffer.h"

/* ***************************   Definitions   **************************** */

const jsonDataBuffer_t DEFAULT_BUFF = {.p_buff = NULL, .size = 0, .id = 0};

/* ****************************   Structures   **************************** */

// Check to ensure the MQTT buffer size is large enough
static_assert((bufferpoolconfigBUFFER_SIZE >= DATA_BUFFER_BUFFER_SIZE),
              "MQTT buffer to small to hold entire JSON payload.");

typedef struct
{
    volatile bool busy;
    char buffer[DATA_BUFFER_BUFFER_SIZE];
} jsonBufferObj_t;

/* ***********************   File Scope Variables   *********************** */

static jsonBufferObj_t g_buffers[DATA_BUFFER_NUM_BUFFERS] = {0};

static SemaphoreHandle_t buffers_counting_semaphore = NULL;

/* *************************   Public  Functions   ************************ */

//
// Initialize this module
//
void jsonBufferInit(void)
{
    // Initialize the semaphore
    buffers_counting_semaphore =
        xSemaphoreCreateCounting(DATA_BUFFER_NUM_BUFFERS, DATA_BUFFER_NUM_BUFFERS);
    assert(buffers_counting_semaphore != NULL);

    // Free all buffers
    for (int idx = 0; idx < DATA_BUFFER_NUM_BUFFERS; idx++)
    {
        g_buffers[idx].busy = false;
    }
}

// Attempt to take a buffer from the pool
// Function will wait for a buffer to be available until the given timeout
bool jsonBufferGetBuffer(jsonDataBuffer_t *const p_buffer, const int required_size,
                         TickType_t timeout)
{
    assert((p_buffer != NULL) && (required_size <= DATA_BUFFER_BUFFER_SIZE));

    // Attempt to take the semaphore, as it controls access to the buffer resources
    BaseType_t result = xSemaphoreTake(buffers_counting_semaphore, timeout);

    if (result == pdTRUE)
    {
        // Enter critical section to safely examine the buffer list and find a free buffer
        taskENTER_CRITICAL();
        int idx = 0;

        while ((idx < DATA_BUFFER_NUM_BUFFERS) && g_buffers[idx].busy)
        {
            idx++;
        }

        if (idx != DATA_BUFFER_NUM_BUFFERS)
        {
            // Free buffer was found
            // Mark the buffer busy and return the buffer info
            g_buffers[idx].busy = true;
            // Now exit the critical section, as the buffer list is no longer modified
            taskEXIT_CRITICAL();

            // Create a new buffer struct with the proper data and copy it back to the caller
            jsonDataBuffer_t buff =
            {
                .p_buff = &(g_buffers[idx].buffer[0]),
                .size = DATA_BUFFER_BUFFER_SIZE,
                .id = idx
            };
            memcpy(p_buffer, &buff, sizeof(jsonDataBuffer_t));
        }
        else
        {
            // No free buffers found
            taskEXIT_CRITICAL();
            result = pdFAIL;
        }
    }

    return (result == pdTRUE);
}

// Returns a buffer to the pool
void jsonBufferReturnBuffer(jsonDataBuffer_t *const p_buff)
{
    if ((p_buff != NULL) && (p_buff->id < DATA_BUFFER_NUM_BUFFERS) && (p_buff->p_buff != NULL))
    {
        // Enter critical section to safely modify the buffer list
        taskENTER_CRITICAL();
        g_buffers[p_buff->id].busy = false;
        taskEXIT_CRITICAL();

        // Set Buffer pointer to null to indicate the buffer pointer is no longer valid
        memcpy(p_buff, &DEFAULT_BUFF, sizeof(jsonDataBuffer_t));

        // Return the semaphore
        xSemaphoreGive(buffers_counting_semaphore);
    }
    else
    {
        // Invalid buffer struct/data in buffer struct
        assert(false);
    }
}
