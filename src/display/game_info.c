//////////////////////////////////////////////////////////////////////////////
//
//  game_info.c
//
//  Game Info Renderer
//
//  Module description in game_info.h
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Library Includes
#include <SDL.h>

// Project Includes
#include "utility.h"
#include "game_data_parser.h"

// Module Includes
#include "text.h"
#include "game_info.h"

/* ***************************   Definitions   **************************** */

#define NORMAL_FONT_SIZE        18


#define VERTICAL_TEXT_OFFSET     (NORMAL_FONT_SIZE + 4)
#define PIX_PER_CHAR    (NORMAL_FONT_SIZE - (NORMAL_FONT_SIZE / 4))

// Horizontal spacing between games
#define GAME_SPACING    200

// NOTE: This size should be sync'd with the size of the downloaded image
#define SELECTED_IMAGE_SIZE_W   480
#define SELECTED_IMAGE_SIZE_H   270

#define UNSELECTED_IMAGE_SIZE_W   270
#define UNSELECTED_IMAGE_SIZE_H   154

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static void gameDisplayGame(int x, int y, SDL_Renderer *renderer, gameObject_t *game_obj);

/* ***********************   File Scope Variables   *********************** */

// Just some sample data with a null image to display
static const gameData_t sample_data =
{
    .date_str = "2018-12-31T11:59:59",
    .home_team_name_str = "Chicaco White Sox",
    .away_team_name_str = "Cleveland Indians",
    .home_team_score_str = "77",
    .away_team_score_str = "2",
    .detailed_state_str = "FINAL",
    .p_img_data = NULL
};

/* *************************   Public  Functions   ************************ */

//
void gameDisplayGames(const gameDataNode_t *p_game_node)
{
    // Previous nodes will be displayed to the left

    // Current node displayed in the middle

    // Next nodes will displayed to the right

}

/* *************************   Private Functions   ************************ */

static void gameDisplayGame(const int x, const int y, SDL_Renderer *renderer, gameObject_t *game_obj)
{
    assert(game_obj != NULL && renderer != NULL);

    // Display date above the image
    textObject_t date = textInitObj(game_obj->game_data->date_str, NORMAL_FONT_SIZE, x,y);
    textDisplay(x, y, renderer, &date);

    // Display image


    // Create other text elements
    textObject_t game_state = textInitObj(game_obj->game_data->detailed_state_str, NORMAL_FONT_SIZE, x, y);
    textObject_t home_team_name = textInitObj(game_obj->game_data->home_team_name_str, NORMAL_FONT_SIZE, x, y);
    textObject_t away_team_name = textInitObj(game_obj->game_data->away_team_name_str, NORMAL_FONT_SIZE, x, y);
    textObject_t home_team_score = textInitObj(game_obj->game_data->home_team_score_str, NORMAL_FONT_SIZE, x, y);
    textObject_t away_team_score = textInitObj(game_obj->game_data->away_team_score_str, NORMAL_FONT_SIZE, x, y);

    int score_offset = (MAX(strlen(sample_data.home_team_name_str), strlen(sample_data.home_team_name_str)) * PIX_PER_CHAR);

    // Offset things in the y direction
    textDisplay(x, y + HOME_OFFSET, renderer, &home_team_name);
    textDisplay(x, y + AWAY_OFFSET, renderer, &away_team_name);
    textDisplay(x + score_offset, y + HOME_OFFSET, renderer, &home_team_score);
    textDisplay(x + score_offset, y + AWAY_OFFSET, renderer, &away_team_score);
    textDisplay(x, y + 100, renderer, &game_state);
}
