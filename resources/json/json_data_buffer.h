//////////////////////////////////////////////////////////////////////////////
//
//  json_data_buffer.h
//
//  JSON Data Buffer
//
//  Copyright Fortune Brands Home & Security, 2018
//  All Rights Reserved.  This is a confidential intellectual property work
//  of Fortune Brands Home & Security and is not to be copied or transmitted to
//  other parties without the express written permission of Fortune Brands
//  Home & Security.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef JSON_DATA_BUFFER_H
#define JSON_DATA_BUFFER_H

/* ***************************    Includes     **************************** */

/* ***************************   Definitions   **************************** */

// Number of buffers in the pool
#define DATA_BUFFER_NUM_BUFFERS     2

// Size of each of the buffers
//
// NOTE: As of 1-OCT-2018, AWS meters MQTT messages at a size of 5KB.
// NOTE: https://aws.amazon.com/iot-core/pricing/additional-details/#Connectivity
#define DATA_BUFFER_BUFFER_SIZE     2500

#define DATA_BUFFER_INIT { .p_buff = NULL, .size = 0, .id = 0 }

#define DATA_BUFFER_INIT { .p_buff = NULL, .size = 0, .id = 0 }

typedef struct
{
    char *p_buff;
    int size;
    int id;  // Used for internal bookkeeping
} jsonDataBuffer_t;

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

void jsonBufferInit(void);
bool jsonBufferGetBuffer(jsonDataBuffer_t *const p_buffer, const int required_size,
                         const TickType_t timeout);
void jsonBufferReturnBuffer(jsonDataBuffer_t *const p_buff);

#endif /* JSON_DATA_BUFFER_H */
