/*****************************************************************************
 * @file    game_data_parser.h
 * @brief   Source code .h file for Game Data Parser module
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

#ifndef GAME_DATA_PARSER_H
#define GAME_DATA_PARSER_H

/**
 * @addtogroup game_data_parser
 * @{
 */
/* ***************************    Includes     **************************** */

#include "shared_data_types.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

typedef struct gameDataNode gameDataNode_t;

// Linked list node
struct gameDataNode
{
    gameDataNode_t *next;
    gameDataNode_t *prev;
    gameData_t *p_data;
};

/* ***********************   Function Prototypes   ************************ */

gameDataNode_t *gameDataParserGatherData(const char *const p_json_url);

/** @} */

#endif /* GAME_DATA_PARSER_H */
