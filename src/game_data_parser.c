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

// Std
#include <stdbool.h>
#include <string.h>

// Libs
#include "jsmn/jsmn.h"

// Module
#include "curl_lib.h"
#include "game_data_parser.h"

/* ***************************   Definitions   **************************** */

#define DEFAULT_NUM_TOKENS_TO_ALLOC 2500

/* ****************************   Structures   **************************** */

typedef struct gameDataNode gameDataNode_t;

struct gameDataNode
{
    gameDataNode_t *next;
    gameData_t *p_data;
};

typedef struct
{
    int num_tokens;      // Number of tokens in the dataset
    jsmntok_t *p_tokens; // Pointer to the tokens
} jsmnTokenizationData_t;

/* ***********************   Function Prototypes   ************************ */

static bool gameDataTokenizeJson(const httpDataBuffer_t *const p_buff, jsmnTokenizationData_t *const p_token_data);

/* ***********************   File Scope Variables   *********************** */

/* ****************************   BEGIN CODE   **************************** */

/* *************************   Public  Functions   ************************ */

void gameDataParserInit(void)
{
    // take in a handle to a semaphore to indicate that parsing is complete?
}

//
void gameDataParserGatherData(const char *const p_json_url)
{
    // Take a URL and get the JSON data
    httpDataBuffer_t json_data_buff;
    curlLibGetData(&json_data_buff, p_json_url);

    // Tokenize the JSON data
    jsmnTokenizationData_t token_data;
    gameDataTokenizeJson(&json_data_buff, &token_data);

    // Seek to the "dates.games" array by searching the tokens
    // NOTE: The reason this needs to be done is because the JSON deserialization only
    // NOTE: operates deserializes json with objects as it's root.

    // Parse each game object, creating a linked list of game objects

    // Mutex lock the resources to ensure no modification in the middle of parsing (might not be necessary based on the design)

    // Progressively allocate and parse the objects out of the games array

    // Unlock resources

    // Free JSON buffer

    // Return a status indicating completed or failed

    // Hand the linked list of game objects back to the caller
}

/* *************************   Private Functions   ************************ */

// Utilizes JSMN JSON tokenizer to tokenize the json data
static bool gameDataTokenizeJson(const httpDataBuffer_t *const p_buff,
                                 jsmnTokenizationData_t *const p_token_data)
{
    // Init parser and token data struct
    jsmn_parser parser;
    jsmn_init(&parser);
    memset(p_token_data, 0, sizeof(jsmnTokenizationData_t));

    // Malloc a token list to store the tokens
    int num_tokens = DEFAULT_NUM_TOKENS_TO_ALLOC;
    jsmntok_t *p_tokens = malloc(sizeof(jsmntok_t) * num_tokens);

    bool tokenization_failed = false;
    if (p_tokens != NULL)
    {
        // Attempt to tokenize the json data, increasing the number of tokens available if needed.
        do
        {
            int jsmn_result = jsmn_parse(&parser, p_buff->p_buffer, p_buff->content_length, p_tokens, num_tokens);
            if (jsmn_result != 0 && jsmn_result == JSMN_ERROR_NOMEM)
            {
                // Double the number of tokens and try again to tokenize
                num_tokens *= 2;
                tokenization_failed = !(realloc(p_tokens, num_tokens));
            }
            else
            {
                // All other JSMN errors are unrecoverable at this point and cause the tokenization to fail
                tokenization_failed = true;
            }
        } while (!tokenization_failed);
    }
    else
    {
        tokenization_failed = true;
    }

    return !(tokenization_failed);
}

// NOTE: This could enforce that "games" is a child of "dates", since "dates" is the child of the parent object
// NOTE: but for now this lazy approach should work.
//
// NOTE: This could be turned into a generic function that could find any array/object given a key and
// NOTE: possibly a path
// Searches the token list for the named array.
// Returns an index of the token in the list for the corresponding array
static int gameDataFindNamedArray(const jsmnTokenizationData_t *const p_tok_data, const httpDataBuffer_t *const p_buff, const char *const key_of_array_str)
{
    bool at_buffer_end = false;
    bool matching_token_found = false;

    for (int idx = 0; ((idx < p_tok_data->num_tokens) && !matching_token_found); idx++)
    {
        // The token must be at least beyond the first token, otherwise it can't be a named array
        if (p_tok_data->p_tokens[idx].type == JSMN_ARRAY && idx > 0)
        {
            // Check the key token, which is the token before the current, to see if the string matches
            const jsmntok_t *const p_tok = &p_tok_data->p_tokens[idx - 1];
            // Catch access that will be out of bounds; This should never happen
            assert(p_buff->content_length > (p_tok->start + p_tok->size));

            const char* const key_str = p_buff->p_buffer + p_tok->start;

            matching_token_found = (p_tok->type == JSMN_STRING) &&
                (strncmp(key_str, key_of_array_str, p_tok->size) == 0);
        }
    }
}

// Expects to be passed a token belonging to the beginning of the object inside the named "game" array

static void gameDataDeserializeGame(void)
{
}
