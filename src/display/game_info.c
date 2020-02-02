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
#include <assert.h>

// Library Includes
#include <SDL.h>

// Project Includes
#include "utility.h"
#include "game_data_parser.h"

// Module Includes
#include "text.h"
#include "image.h"
#include "game_info.h"

/* ***************************   Definitions   **************************** */

#define NORMAL_FONT_SIZE 18

#define VERTICAL_TEXT_OFFSET (NORMAL_FONT_SIZE + 4)
#define PIX_PER_CHAR (NORMAL_FONT_SIZE - (NORMAL_FONT_SIZE / 4))

// Horizontal spacing between games
#define GAME_SPACING 200

// NOTE: This size should be sync'd with the size of the downloaded image
#define SELECTED_IMAGE_SIZE_W 480
#define SELECTED_IMAGE_SIZE_H 270

#define UNSELECTED_IMAGE_SIZE_W 270
#define UNSELECTED_IMAGE_SIZE_H 154

/* ****************************   Structures   **************************** */

typedef struct
{
    int pos_x;
    int pos_y;
    bool selected;
    const gameData_t *game_data;
} gameObject_t;

typedef struct
{
    int pos_x;
    int pos_y;
    bool selected;
    drawableObj_t date;
    drawableObj_t game_state;
    drawableObj_t home_team_name;
    drawableObj_t away_team_name;
    drawableObj_t home_team_score;
    drawableObj_t away_team_score;
    drawableObj_t thumb;
} gameDisplayData_t;

typedef struct gameDisplayNode gameDisplayNode_t;

// Linked list node
struct gameDisplayNode
{
    gameDisplayNode_t *next;
    gameDisplayNode_t *prev;
    gameDisplayData_t *p_data;
};

/* ***********************   Function Prototypes   ************************ */

static void gameDisplayGame(SDL_Renderer *renderer, const gameObject_t *game_obj);

static void gameDataCreate(const gameObject_t *game_obj);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

//
void gameDisplayGames(const gameDataNode_t *p_game_node, SDL_Renderer *renderer)
{
    int width;
    int height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    // Current node displayed in the middle

    static gameObject_t game;
    static bool inited = false;
    if (!inited)
    {
        int starting_x_coord = (width / 2) - (SELECTED_IMAGE_SIZE_W / 2);
        int y_coord = (height / 2) - (SELECTED_IMAGE_SIZE_H / 2);
        game = (gameObject_t){.game_data = p_game_node->p_data, .selected = false, .pos_x = starting_x_coord, .pos_y = y_coord};
        gameDataCreate(&game);
        inited = true;
    }
    else
    {
        game.pos_x += 5;
    }

    gameDisplayGame(renderer, &game);

    // Previous nodes will be displayed to the left

    // Next nodes will displayed to the right
}

/* *************************   Private Functions   ************************ */

static drawableObj_t date;
static drawableObj_t game_state;
static drawableObj_t home_team_name;
static drawableObj_t away_team_name;
static drawableObj_t home_team_score;
static drawableObj_t away_team_score;
static drawableObj_t thumb;

static void gameDisplayGame(SDL_Renderer *renderer, const gameObject_t *game_obj)
{
    assert(game_obj != NULL && renderer != NULL);

    int x = game_obj->pos_x;
    int y = game_obj->pos_y;

    // Display date above the image
    date.draw(&date, x, y, renderer);
    y += (VERTICAL_TEXT_OFFSET);

    // Display image
    thumb.draw(&thumb, x, y, renderer);

    // Shift y by image size
    y += SELECTED_IMAGE_SIZE_H;

    int score_offset = (MAX(strlen(game_obj->game_data->home_team_name_str), strlen(game_obj->game_data->home_team_name_str)) * PIX_PER_CHAR);

    // Offset things in the y direction
    y += (VERTICAL_TEXT_OFFSET);
    home_team_name.draw(&home_team_name, x, y, renderer);
    home_team_score.draw(&home_team_score, x + score_offset, y, renderer);
    y += (VERTICAL_TEXT_OFFSET);
    away_team_name.draw(&away_team_name, x, y, renderer);
    away_team_score.draw(&away_team_score, x + score_offset, y, renderer);
    y += (VERTICAL_TEXT_OFFSET);
    game_state.draw(&game_state, x, y, renderer);
}

static void gameDataCreate(const gameObject_t *game_obj)
{
    int x = 0;
    int y = 0;
    date = textInitObj(game_obj->game_data->date_str, NORMAL_FONT_SIZE, x, y);

    thumb = imgInitObjBuff(x, y, game_obj->game_data->p_img_data->p_buffer, game_obj->game_data->p_img_data->content_length);
    // Create other text elements
    game_state = textInitObj(game_obj->game_data->detailed_state_str, NORMAL_FONT_SIZE, x, y);
    home_team_name = textInitObj(game_obj->game_data->home_team_name_str, NORMAL_FONT_SIZE, x, y);
    away_team_name = textInitObj(game_obj->game_data->away_team_name_str, NORMAL_FONT_SIZE, x, y);
    home_team_score = textInitObj(game_obj->game_data->home_team_score_str, NORMAL_FONT_SIZE, x, y);
    away_team_score = textInitObj(game_obj->game_data->away_team_score_str, NORMAL_FONT_SIZE, x, y);
}
