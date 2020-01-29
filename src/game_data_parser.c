/*****************************************************************************
 * @file    game_data_parser.c
 * @brief   Source code .c file for Game Data Parser module
 * @date    2020-JAN-29
 *
 * @note <insert notes>
 *
 * @ingroup game_data_parser
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
 * @addtogroup game_data_parser
 * @{
 */

/* ***************************    Includes     **************************** */

// Module
#include "game_data_parser.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

typedef struct gameDataNode gameDataNode_t;

struct gameDataNode
{
    gameDataNode_t *next;
    gameData_t *p_data;
};

/* ***********************   Function Prototypes   ************************ */

/* ***********************   File Scope Variables   *********************** */

/* ****************************   BEGIN CODE   **************************** */

/* *************************   Public  Functions   ************************ */

void gameDataParserInit(void)
{
    // take in a handle to a semaphore to indicate that parsing is complete?
}

//
void gameDataParserGatherData(const char* const p_json_url)
{
    // Take a URL and get the JSON data

    // Seek to the "dates.games" array

    // Mutex lock the resources to ensure no modification in the middle of parsing

    // Progressively allocate and parse the objects out of the games array

    // Unlock resources

    // Return a status indicating completed or failed
}

/* *************************   Private Functions   ************************ */

/* ************************   Interrupt Functions   *********************** */
