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
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

// Libs
#include "jsmn/jsmn.h"

// App
#include "curl_lib.h"
#include "errors.h"
#include "json_deserializer.h"
#include "utility.h"

// Module
#include "game_data_parser.h"

/* ***************************   Definitions   **************************** */

#define DEFAULT_NUM_TOKENS_TO_ALLOC 2500

/* ****************************   Structures   **************************** */

typedef struct gameDataNode gameDataNode_t;

// Linked list node
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

static bool gameDataTokenizeJson(jsmnTokenizationData_t *const p_token_data, const char *const p_json_buff,
                                 const size_t json_content_length);
static int gameDataFindArray(const jsmnTokenizationData_t *const p_tok_data, const httpDataBuffer_t *const p_buff,
                             const char *const key_of_array_str);
static void gameDataDeserializeGames(const int game_array_idx, const jsmnTokenizationData_t *const p_token_data,
                                     const char *const p_json_buff, const size_t json_content_length);

/* ***********************   File Scope Variables   *********************** */

// Stuct in which the game json data will be parsed into
typedef struct
{
    jsonStr_t game_date;
    jsonStr_t home_team_name;
    jsonStr_t away_team_name;
    jsonStr_t detailed_state;
    uint32_t home_score;
    uint32_t away_score;
    jsonStr_t img_url;
} gameDataObj_t;

// ******* BEGIN JSON parsing structures *******

static const jsonKeyValue_t g_root_obj_data[] =
    {
        {
            .key_str = "gameDate",
            .c_type = E_JSON_C_STR_PTR,
            .struct_member_offset = offsetof(gameDataObj_t, game_date),
            .struct_member_size = MEMBER_SIZE(gameDataObj_t, game_date),
            .value_tok_type = JSMN_STRING,
        }};

// Game Object root Keylist definition
static const jsonKeyValueList_t g_root_key_list =
    {
        .p_keys = g_root_obj_data,
        .size = ARRAY_SIZE(g_root_obj_data)};

static const jsonKeyValue_t g_home_team_name_obj_data[] =
    {
        {
            .key_str = "gameDate",
            .c_type = E_JSON_C_STR_PTR,
            .struct_member_offset = offsetof(gameDataObj_t, game_date),
            .struct_member_size = MEMBER_SIZE(gameDataObj_t, game_date),
            .value_tok_type = JSMN_STRING,
        }};

static const jsonKeyValueList_t g_root_key_list =
    {
        .p_keys = g_root_obj_data,
        .size = ARRAY_SIZE(g_root_obj_data)};

// Root object used a game object
static jsonContainer_t g_root_obj =
    {
        .type = E_JSON_OBJECT,
        .children = NULL,
        .num_children = 0, // TODO: This will be 3
        .key_str = NULL,
        .p_key_list = &g_root_key_list,
        .p_deserialized_elements = NULL, // Don't need to know which were deserialized
        .p_elements_to_serialize = NULL, // Serialize all of the data
        .p_data = NULL                   // TODO: Add pointer to game object struct
};

// ******* END JSON parsing structures *******

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
    curlLibBufferInit(&json_data_buff);
    appErrors_t error_status = curlLibGetData(&json_data_buff, p_json_url);

    if (error_status == APPERR_OK)
    {
        // Tokenize the JSON data
        jsmnTokenizationData_t token_data;
        bool result = gameDataTokenizeJson(&token_data, json_data_buff.p_buffer, json_data_buff.content_length);

        if (result)
        {
            // Seek to the "dates.games" array by searching the tokens
            // NOTE: The reason this needs to be done is because the JSON deserialization only
            // NOTE: operates deserializes json with objects as it's root.
            int idx_of_game_data = gameDataFindArray(&token_data, &json_data_buff, "games");

            // Progressively tokenize each object inside the array (making them appear as "root objects"), deserialize them and build the list
            gameDataDeserializeGames(idx_of_game_data, &token_data, json_data_buff.p_buffer, json_data_buff.content_length);
            // This is the easiest approach without modifying the way the deserialization code works
        }
    }

    // Parse each game object, creating a linked list of game objects

    // Mutex lock the resources to ensure no modification in the middle of parsing (might not be necessary based on the design)

    // Progressively allocate and parse the objects out of the games array

    // Unlock resources

    // Free JSON buffer (since it's no longer needed)

    // Return a status indicating completed or failed

    // Hand the linked list of game objects back to the caller

    curlLibFreeData(&json_data_buff);
}

void gameDataParserFreeGameList(gameDataNode_t *p_list)
{
    // Traverse through the linked list freeing each bit that was malloc'd
}

/* *************************   Private Functions   ************************ */

// Utilizes JSMN JSON tokenizer to tokenize the json data
static bool gameDataTokenizeJson(jsmnTokenizationData_t *const p_token_data,
                                 const char *const p_json_buff,
                                 const size_t json_content_length)
{
    // Init parser and token data struct
    jsmn_parser parser;
    memset(p_token_data, 0, sizeof(jsmnTokenizationData_t));

    // Malloc a token list to store the tokens
    p_token_data->num_tokens = DEFAULT_NUM_TOKENS_TO_ALLOC;
    p_token_data->p_tokens = calloc(p_token_data->num_tokens, sizeof(jsmntok_t));

    bool tokenization_failed = false;
    if (p_token_data->p_tokens != NULL)
    {
        // Attempt to tokenize the json data, increasing the number of tokens available if needed.
        int jsmn_result;
        do
        {
            jsmn_init(&parser);
            jsmn_result = jsmn_parse(&parser, p_json_buff, json_content_length, p_token_data->p_tokens, p_token_data->num_tokens);
            if (jsmn_result == JSMN_ERROR_NOMEM)
            {
                // Double the number of tokens and try again to tokenize
                p_token_data->num_tokens *= 2;
                free(p_token_data->p_tokens);
                p_token_data->p_tokens = calloc(p_token_data->num_tokens, sizeof(jsmntok_t));
                //tokenization_failed = (realloc(p_tokens, num_tokens * sizeof(jsmntok_t))) == NULL;
                tokenization_failed = (p_token_data->p_tokens == NULL);
            }
            else if (jsmn_result < 0)
            {
                // All other JSMN errors are unrecoverable at this point and cause the tokenization to fail
                tokenization_failed = true;
            }
            else
            {
                // The result is the number of actual parsed tokens.
                p_token_data->num_tokens = jsmn_result;
            }
        } while ((!tokenization_failed) && (jsmn_result < 0));
    }
    else
    {
        tokenization_failed = true;
    }

    return !(tokenization_failed);
}

// Frees the token list that was allocated
static void gameDataFreeTokenData(jsmnTokenizationData_t *const p_token_data)
{
    free(p_token_data->p_tokens);
}

// NOTE: This could enforce that "games" is a child of "dates", since "dates" is the child of the parent object
// NOTE: but for now this lazy approach should work (barring no second "games" array shows up in the data).
//
// NOTE: This could be turned into a generic function that could find any array/object given a key and
// NOTE: possibly a path. (i.e. given the string "dates.games", it could seek to the element with that key
// NOTE: and return the type (array, object, string or primitive) and the token index). It could potentially
// NOTE: even take an expected type to ensure the caller gets exactly what is expected, instead of having
// NOTE: to re-query.
// Searches the token list for the named array.
// Returns an index of the token in the list for the corresponding array
static int gameDataFindArray(const jsmnTokenizationData_t *const p_tok_data, const httpDataBuffer_t *const p_buff,
                             const char *const key_of_array_str)
{
    bool at_buffer_end = false;
    bool matching_token_found = false;

    // This will be the index where the game data is found
    int token_idx = 0;
    while ((token_idx < p_tok_data->num_tokens) && !matching_token_found)
    {
        // The token must be at least beyond the first token, otherwise it can't be a named array
        if (p_tok_data->p_tokens[token_idx].type == JSMN_ARRAY && token_idx > 0)
        {
            // Check the key token, which is the token before the current, to see if the string matches
            const jsmntok_t *const p_tok = &p_tok_data->p_tokens[token_idx - 1];
            // Catch access that will be out of bounds; This should never happen
            assert(p_buff->content_length > (p_tok->start + p_tok->size));

            const char *const key_str = p_buff->p_buffer + p_tok->start;
            matching_token_found = (p_tok->type == JSMN_STRING) &&
                                   (strncmp(key_str, key_of_array_str, p_tok->size) == 0);
        }

        if (!matching_token_found)
        {
            token_idx++;
        }
    }

    // TODO: with a matching token, it's worthwhile to return the size of the array (size attrib in the token data)

    return (matching_token_found ? token_idx : -1);
}

static void gameDataDeserializeGames(const int game_array_idx, const jsmnTokenizationData_t *const p_token_data,
                                     const char *const p_json_buff, const size_t json_content_length)
{
    // Check the next token, it should be an object token with a parent idx equal to game_array_idx
    int next_game_tok_idx = (game_array_idx + 1);
    if ((next_game_tok_idx < p_token_data->num_tokens) &&
        (p_token_data->p_tokens[next_game_tok_idx].parent == game_array_idx))
    {
        jsmntok_t *p_game_obj_tok = &p_token_data->p_tokens[next_game_tok_idx];

        // If this is true, tokenize this object, create a new buffer that points to the object start
        // then pass it off to be deserialized
        jsmnTokenizationData_t game_obj_token_data;
        const char *const obj_start_char = (p_json_buff + p_game_obj_tok->start);
        const int obj_len = p_game_obj_tok->end - p_game_obj_tok->start;
        bool result = gameDataTokenizeJson(&game_obj_token_data, obj_start_char, obj_len);

        // TODO: Deserialize the game data

        // Free the JSON tokens after deserialization is finished
        gameDataFreeTokenData(&game_obj_token_data);
    }
}

// Expects to be passed a token belonging to the beginning of the object inside the named "game" array
static void gameDataDeserializeGame(void)
{
}

// TODO: Move JSON related items out of here

// Searches for an expected object (based on a dot notation of the reference) in the json
// data and returns a JSMN token index of that data
// NOTE: There is no support of wildcards ('*' or '?') in the search
static int jsonSearchForElement(const jsmnTokenizationData_t *const p_tok_data, const char *const json_ref, const jsmntok_t type)
{
    // Check for malformed reference
    assert((*json_ref != '\0') && (*json_ref != '.'));

    int current_char_idx = 0;
    int last_element_start = 0;
    while (json_ref[current_char_idx] != '\0')
    {
        // Look to see if current character is a '.'
        if (json_ref[current_char_idx] != '.')
        {
            // Found the end of the current reference

            // Form a string, and search for a token with the required parent
        }
        else
        {
            // Assume is part of the next reference and keep going
        }
        current_char_idx++;
    }
}

static int jsonFindElement(const jsmnTokenizationData_t *const p_tok_data, const char *const key_str, const jsmntok_t type, const int parent_idx)
{
    int idx = 0;
    bool key_found = false;
    while (idx < p_tok_data->num_tokens && !key_found)
    {
        // Parent object matches and the data is a string
        if (p_tok_data->p_tokens[idx].parent == parent_idx &&
            p_tok_data->p_tokens[idx].type == JSMN_STRING)
        {
            // Check to see if key string matches to the JSON string data

            // If true, check to see the next token type matches the required type, if so, object is
            // otherwise keep going
        }

        ++idx;
    }

    // If idx is the number of elements, element wasn't found in the list.
    ASSERT(idx != num_of_settings);
}

// static int jsonFindElement(const jsmnTokenizationData_t *const p_tok_data, const httpDataBuffer_t *const p_buff,
//                                   const char *const key_of_array_str)
// {
//     bool at_buffer_end = false;
//     bool matching_token_found = false;

//     // This will be the index where the game data is found
//     int token_idx = 0;
//     while ((token_idx < p_tok_data->num_tokens) && !matching_token_found)
//     {
//         // The token must be at least beyond the first token, otherwise it can't be a named array
//         if (p_tok_data->p_tokens[token_idx].type == JSMN_ARRAY && token_idx > 0)
//         {
//             // Check the key token, which is the token before the current, to see if the string matches
//             const jsmntok_t *const p_tok = &p_tok_data->p_tokens[token_idx - 1];
//             // Catch access that will be out of bounds; This should never happen
//             assert(p_buff->content_length > (p_tok->start + p_tok->size));

//             const char *const key_str = p_buff->p_buffer + p_tok->start;
//             matching_token_found = (p_tok->type == JSMN_STRING) &&
//                                    (strncmp(key_str, key_of_array_str, p_tok->size) == 0);
//         }

//         if (!matching_token_found)
//         {
//             token_idx++;
//         }
//     }

//     // TODO: with a matching token, it's worthwhile to return the size of the array (size attrib in the token data)

//     return (matching_token_found ? token_idx : -1);
// }
