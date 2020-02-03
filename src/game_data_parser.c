//////////////////////////////////////////////////////////////////////////////
//
//  game_data_parser.c
//
//  Game Data Parser
//
//  Module description in game_data_parser.h
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
#include "utility.h"
#include "json_deserialization.h"

// Module
#include "game_data_parser.h"

/* ***************************   Definitions   **************************** */

#define DEFAULT_NUM_TOKENS_TO_ALLOC 2500

/* ****************************   Structures   **************************** */

// Stuct in which the game json data will be parsed into
typedef struct
{
    jsonStr_t game_date;      // gameDate
    jsonStr_t home_team_name; // teams.home.team.name
    jsonStr_t away_team_name; // teams.away.team.name
    jsonStr_t detailed_state; // status.detailedState
    uint32_t home_score;      // teams.home.score
    uint32_t away_score;      // teams.away.score
    jsonStr_t img_url;        // content.editorial.recap.home.photo.cuts.960x540.src
} gameDataObj_t;

/* ***********************   Function Prototypes   ************************ */

static bool gameDataTokenizeJson(jsmnTokenizationData_t *const p_token_data, const char *const p_json_buff,
                                 const size_t json_content_length);
static int gameDataFindArray(const jsmnTokenizationData_t *const p_tok_data, const httpDataBuffer_t *const p_buff,
                             const char *const key_of_array_str);
static gameDataNode_t *gameDataDeserializeGames(const int game_array_idx, const jsmnTokenizationData_t *const p_token_data,
                                     const char *const p_json_buff, const size_t json_content_length);
static gameDataNode_t *gameDataDeserializeGame(const gameDataObj_t *p_game_data_obj, gameDataNode_t *p_prev_node);

/* ***********************   File Scope Variables   *********************** */

const jsonKeyValue_t g_list_of_game_obj_values[] =
    {
        {.key_str = "gameDate",
         .c_type = E_JSON_C_STR_PTR,
         .value_tok_type = JSMN_STRING,
         .struct_member_offset = offsetof(gameDataObj_t, game_date),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, game_date)},
        {.key_str = "teams.home.team.name",
         .c_type = E_JSON_C_STR_PTR,
         .value_tok_type = JSMN_STRING,
         .struct_member_offset = offsetof(gameDataObj_t, home_team_name),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, home_team_name)},
        {.key_str = "teams.away.team.name",
         .c_type = E_JSON_C_STR_PTR,
         .value_tok_type = JSMN_STRING,
         .struct_member_offset = offsetof(gameDataObj_t, away_team_name),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, away_team_name)},
        {.key_str = "teams.home.score",
         .c_type = E_JSON_C_UINT32,
         .value_tok_type = JSMN_PRIMITIVE,
         .struct_member_offset = offsetof(gameDataObj_t, home_score),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, home_score)},
        {.key_str = "teams.away.score",
         .c_type = E_JSON_C_UINT32,
         .value_tok_type = JSMN_PRIMITIVE,
         .struct_member_offset = offsetof(gameDataObj_t, away_score),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, away_score)},
        {.key_str = "status.detailedState",
         .c_type = E_JSON_C_STR_PTR,
         .value_tok_type = JSMN_STRING,
         .struct_member_offset = offsetof(gameDataObj_t, detailed_state),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, detailed_state)},
        {.key_str = "content.editorial.recap.home.photo.cuts.480x270.src",
         .c_type = E_JSON_C_STR_PTR,
         .value_tok_type = JSMN_STRING,
         .struct_member_offset = offsetof(gameDataObj_t, img_url),
         .struct_member_size = MEMBER_SIZE(gameDataObj_t, img_url)},
};

/* ****************************   BEGIN CODE   **************************** */

/* *************************   Public  Functions   ************************ */

// Parses the game data at the URL provided and returns a linked list of game data
gameDataNode_t *gameDataParserGatherData(const char *const p_json_url)
{
    // Take a URL and get the JSON data
    httpDataBuffer_t json_data_buff;
    curlLibBufferInit(&json_data_buff);
    appErrors_t error_status = curlLibGetData(&json_data_buff, p_json_url);

    gameDataNode_t *p_first_node = NULL;
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
            p_first_node = gameDataDeserializeGames(idx_of_game_data, &token_data, json_data_buff.p_buffer, json_data_buff.content_length);
            // This is the easiest approach without modifying the way the deserialization code works
        }
    }
    curlLibFreeData(&json_data_buff);

    // Hand the linked list of game objects back to the caller
    return p_first_node;
}

// Free the list of game nodes, starting with the first
// WARN: The first node must be passed into the function
void gameDataParserGameListDestroy(gameDataNode_t *p_list)
{
    // Not allowed to start freeing the list anywhere except the first node!
    assert(p_list->prev == NULL);

    // Traverse through the linked list freeing each bit that was malloc'd until
    // the next node is null
    gameDataNode_t *p_current_node = p_list;
    while(p_current_node != NULL)
    {
        // Free the image data first
        curlLibFreeData(p_current_node->p_data->p_img_data);

        // Free all the members of the game data
        free(p_current_node->p_data->home_team_name_str);
        free(p_current_node->p_data->away_team_name_str);
        free(p_current_node->p_data->detailed_state_str);
        free(p_current_node->p_data->home_team_score_str);
        free(p_current_node->p_data->away_team_score_str);

        // Free the game data struct
        free(p_current_node->p_data);

        // Save the next node for traversal
        gameDataNode_t* p_next_node = p_current_node->next;

        // Free the linked list node
        free(p_current_node);

        // Traverse to the next node
        p_current_node = p_next_node;
    }
}

/* *************************   Private Functions   ************************ */

// Utilizes jsmn JSON tokenizer to tokenize the json data
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

// Creates a linked list of game data and hands back the first item in the list to the caller
static gameDataNode_t *gameDataDeserializeGames(const int game_array_idx, const jsmnTokenizationData_t *const p_token_data,
                                                const char *const p_json_buff, const size_t json_content_length)
{
    // Determine the number of games by the size of the ARRAY token
    int num_games = p_token_data->p_tokens[game_array_idx].size;

    // Declare pointer that will point to the linked list
    gameDataNode_t *current_node = NULL;

    // TODO: Iterate through all the games
    int next_game_tok_idx = (game_array_idx + 1);
    for (int idx = 0; idx < num_games; idx++)
    {

        // Check the next token, it should be an object token with a parent idx equal to game_array_idx
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

            // Find the value token that matches the desired element and deserialize the game data
            gameDataObj_t game_data_deserialized;
            memset(&game_data_deserialized, 0, sizeof(gameDataObj_t));
            for (int jdx = 0; jdx < ARRAY_SIZE(g_list_of_game_obj_values); jdx++)
            {
                const jsonKeyValue_t *const p_value_data = &g_list_of_game_obj_values[jdx];
                int value_tok_idx = jsonSearchForElement(&game_obj_token_data, obj_start_char, p_value_data);

                // Game objects should always contain the specified elements. If not, something is wrong.
                assert(value_tok_idx > 0);
                if (value_tok_idx > 0)
                {
                    // Index of token was found, go deserialize into data struct.
                    jsonDeserializeElement(p_value_data, &game_obj_token_data.p_tokens[value_tok_idx],
                                           obj_start_char, &game_data_deserialized);
                }
            }

            // Turn datastruct into linked list object to be returned
            current_node = gameDataDeserializeGame(&game_data_deserialized, current_node);

            // Free the JSON tokens after deserialization is finished
            gameDataFreeTokenData(&game_obj_token_data);
        }

        // Find the next game object
        do
        {
            next_game_tok_idx++;
        } while ((next_game_tok_idx < p_token_data->num_tokens) &&
                 (p_token_data->p_tokens[next_game_tok_idx].parent != game_array_idx));
    }

    // Find the first node
    while(current_node != NULL && current_node->prev != NULL){
        current_node = current_node->prev;
    }

    return current_node;
}

// Expects to be passed a token belonging to the beginning of the object inside the named "game" array
static gameDataNode_t *gameDataDeserializeGame(const gameDataObj_t *p_game_data_obj, gameDataNode_t *p_prev_node)
{
    gameDataNode_t *p_node = malloc(sizeof(gameDataNode_t));

    if (p_node != NULL)
    {
        // Clear out the dataspace to ensure no garbage in the pointers
        memset(p_node, 0, sizeof(gameDataNode_t));

        // Set the previous node to the one specified (even if null)
        p_node->prev = p_prev_node;
        if (p_prev_node != NULL)
        {
            // If there is a previous node, set it to this one
            p_prev_node->next = p_node;
        }
        // Set the next node to null, since there is no next node
        p_node->next = NULL;

        // allocate the space for the game data and start to populate
        p_node->p_data = malloc(sizeof(gameData_t));

        if (p_node->p_data != NULL)
        {
            // Set the variables in the game data that do not need to be malloc'd
            strncpy_s(p_node->p_data->date_str, ARRAY_SIZE(p_node->p_data->date_str), p_game_data_obj->game_date.str, p_game_data_obj->game_date.len);

            // Malloc the strings
            // NOTE: jsonStr_t.len does NOT account for the NULL byte, it is simply the length of the character data (hence the additional byte)
            p_node->p_data->home_team_name_str = malloc(p_game_data_obj->home_team_name.len + 1);
            p_node->p_data->away_team_name_str = malloc(p_game_data_obj->away_team_name.len + 1);
            p_node->p_data->detailed_state_str = malloc(p_game_data_obj->detailed_state.len + 1);

            // Malloc data for the scores
            p_node->p_data->home_team_score_str = malloc(MAX_UINT32_STR_LEN);
            p_node->p_data->away_team_score_str = malloc(MAX_UINT32_STR_LEN);

            // Malloc the space for the image
            char *img_url_str = malloc(p_game_data_obj->img_url.len + 1); // Img URL is going to be used to download the image
            httpDataBuffer_t *p_img_buff = malloc(sizeof(httpDataBuffer_t));

            if (p_node->p_data->home_team_name_str != NULL &&
                p_node->p_data->away_team_name_str != NULL &&
                p_node->p_data->home_team_score_str != NULL &&
                p_node->p_data->away_team_score_str != NULL &&
                p_node->p_data->detailed_state_str != NULL &&
                img_url_str != NULL &&
                p_img_buff != NULL)
            {
                // Copy all the data into the allocated string space.
                strncpy_s(p_node->p_data->home_team_name_str, (p_game_data_obj->home_team_name.len + 1), p_game_data_obj->home_team_name.str, p_game_data_obj->home_team_name.len);
                strncpy_s(p_node->p_data->away_team_name_str, (p_game_data_obj->away_team_name.len + 1), p_game_data_obj->away_team_name.str, p_game_data_obj->away_team_name.len);
                strncpy_s(p_node->p_data->detailed_state_str, (p_game_data_obj->detailed_state.len + 1), p_game_data_obj->detailed_state.str, p_game_data_obj->detailed_state.len);
                strncpy_s(img_url_str, (p_game_data_obj->img_url.len + 1), p_game_data_obj->img_url.str, p_game_data_obj->img_url.len);

                // Convert the scores to strings
                snprintf(p_node->p_data->home_team_score_str, MAX_UINT32_STR_LEN, "%d", p_game_data_obj->home_score);
                snprintf(p_node->p_data->away_team_score_str, MAX_UINT32_STR_LEN, "%d", p_game_data_obj->away_score);

                // Last thing to do is download the image data
                memset(p_img_buff, 0, sizeof(httpDataBuffer_t));
                if (curlLibGetData(p_img_buff, img_url_str) == APPERR_OK)
                {
                    p_node->p_data->p_img_data = p_img_buff;
                }
            }
            else
            {
                // TODO: Malloc failed,  Free all previously malloc'd items

                p_node = NULL;
            }
        }
        else
        {
            // TODO: Malloc failed,  Free all previously malloc'd items

            p_node = NULL;
        }
    }

    return p_node;
}
