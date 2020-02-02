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
#include "errors.h"
#include "utility.h"
#include "game_data_parser.h"

// Module Includes
#include "display.h"
#include "text.h"
#include "image.h"
#include "game_info.h"

/* ***************************   Definitions   **************************** */

#define NORMAL_FONT_SIZE 18

#define VERTICAL_TEXT_OFFSET (NORMAL_FONT_SIZE + 4)
#define PIX_PER_CHAR (NORMAL_FONT_SIZE - (NORMAL_FONT_SIZE / 4))

// Horizontal spacing between games
#define GAME_SPACING 20

// NOTE: This size should be sync'd with the size of the downloaded image
#define SELECTED_IMAGE_SIZE_W 480
#define SELECTED_IMAGE_SIZE_H 270

#define UNSELECTED_IMAGE_SIZE_W 360
#define UNSELECTED_IMAGE_SIZE_H 202

/* ****************************   Structures   **************************** */

typedef struct
{
    int pos_x;
    int pos_y;
    bool selected;
    int score_offset;
    drawableObj_t date;
    drawableObj_t game_state;
    drawableObj_t home_team_name;
    drawableObj_t away_team_name;
    drawableObj_t home_team_score;
    drawableObj_t away_team_score;
    drawableObj_t thumb;
} gameDisplayObj_t;


typedef struct gameDisplayNode gameDisplayNode_t;

// Linked list node
struct gameDisplayNode
{
    gameDisplayNode_t *next;
    gameDisplayNode_t *prev;
    gameDisplayObj_t *p_data;
};



/* ***********************   Function Prototypes   ************************ */

static void gameDisplayGame(SDL_Renderer *renderer, gameDisplayObj_t *game);

static void gameDisplayEventHandler(const SDL_Event *p_event);

static const gameDisplayNode_t *gameFindSelectedNode(const gameDisplayNode_t *p_node_list);

static gameDisplayNode_t *gameDisplayObjListCreate(const gameDataNode_t *p_node);
static void gameDisplayObjListDestroy(const gameDisplayNode_t *p_node);

static gameDisplayObj_t *gameDisplayObjCreate(const gameData_t *p_game_data);
static gameDisplayObjDestroy(gameDisplayObj_t *p_obj);

/* ***********************   File Scope Variables   *********************** */

// Holds the single instance of game display structs
static gameDisplayNode_t *g_game_object_list = NULL;

/* *************************   Public  Functions   ************************ */

// Creates the game list
displayEventHandlerFcn_t *gameDisplayInit(const gameDataNode_t *p_node)
{
    // TODO: This is a cop-out. This should function should destroy the current list and recreate it.
    // TODO: Time pending, come back and fix this.
    assert(g_game_object_list == NULL);

    g_game_object_list = gameDisplayObjListCreate(p_node);
    return gameDisplayEventHandler;
}


//
void gameDisplayGames(SDL_Renderer *renderer)
{
    assert(g_game_object_list != NULL && renderer != NULL);

    int width;
    int height;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    // Get the selected node
    const gameDisplayNode_t *p_selected_node = gameFindSelectedNode(g_game_object_list);

    // Current node displayed in the middle
    int starting_x_coord = (width / 2) - (SELECTED_IMAGE_SIZE_W / 2);
    int y_coord = (height / 2) - (SELECTED_IMAGE_SIZE_H / 2);
    p_selected_node->p_data->pos_x = starting_x_coord;
    p_selected_node->p_data->pos_y = y_coord;

    gameDisplayGame(renderer, p_selected_node->p_data);

    // Change y for non-selected games
    y_coord = (height / 2) - (UNSELECTED_IMAGE_SIZE_H / 2);

    // Previous nodes will be displayed to the left
    const gameDisplayNode_t *p_current_node = p_selected_node;
    int next_x_coord = starting_x_coord - (UNSELECTED_IMAGE_SIZE_W + GAME_SPACING);
    while(p_current_node->prev != NULL)
    {
        p_current_node->prev->p_data->pos_x = next_x_coord;
        p_current_node->prev->p_data->pos_y = y_coord;
        gameDisplayGame(renderer, p_current_node->prev->p_data);
        p_current_node = p_current_node->prev;
        next_x_coord -= UNSELECTED_IMAGE_SIZE_W + GAME_SPACING;
    }

    // Next nodes will displayed to the right
    p_current_node = p_selected_node;
    next_x_coord = starting_x_coord + (SELECTED_IMAGE_SIZE_W + GAME_SPACING);
    while(p_current_node->next != NULL)
    {
        p_current_node->next->p_data->pos_x = next_x_coord;
        p_current_node->next->p_data->pos_y = y_coord;
        gameDisplayGame(renderer, p_current_node->next->p_data);
        p_current_node = p_current_node->next;
        next_x_coord += UNSELECTED_IMAGE_SIZE_W + GAME_SPACING;
    }
}




/* *************************   Private Functions   ************************ */


// Function to handle keypresses from the SDL layer (left and right)
static void gameDisplayEventHandler(const SDL_Event *p_event)
{
    // Module only responds to keypresses
    if(p_event->type == SDL_KEYDOWN)
    {
        switch (p_event->key.keysym.sym)
        {
        case SDLK_RIGHT:
            {
                gameDisplayNode_t *p_node = gameFindSelectedNode(g_game_object_list)->next;
                if(p_node != NULL)
                {
                    p_node->p_data->selected = true;
                    p_node->prev->p_data->selected = false;
                }
            }
            break;
        case SDLK_LEFT:
            {
                gameDisplayNode_t *p_node = gameFindSelectedNode(g_game_object_list)->prev;
                if(p_node != NULL)
                {
                    p_node->p_data->selected = true;
                    p_node->next->p_data->selected = false;
                }
            }
            break;

        default:
            break;
        }
    }
}

// Given a linked list of game data nodes, a linked list of game display objects will be created
// NOTE: The first game data node should be passed in
static gameDisplayNode_t *gameDisplayObjListCreate(const gameDataNode_t *p_node)
{
    // Ensure the passed in node is the first node
    assert(p_node->prev == NULL);

    gameDisplayNode_t *p_list = NULL;

    const gameDataNode_t *curr_data_node = p_node;
    bool malloc_fail = false;
    while(curr_data_node != NULL && !malloc_fail)
    {
        gameDisplayNode_t *curr_disp_node = malloc(sizeof(gameDisplayNode_t));
        if(curr_disp_node != NULL)
        {
            // Clear out the data space
            memset(curr_disp_node, 0, sizeof(gameDisplayNode_t));

            // Link the previous node to the last one created (even if it's the first one)
            // as long as p_list is set to NULL, this is fine
            curr_disp_node->prev = p_list;

            curr_disp_node->p_data = gameDisplayObjCreate(curr_data_node->p_data);
            if(curr_disp_node->p_data == NULL)
            {
                malloc_fail = true;
            }
        }
        else
        {
            malloc_fail = true;
        }

        // If not the first node created, link them up
        if(p_list != NULL)
        {
            p_list->next = curr_disp_node;
        }

        // Advance to the next nodes
        p_list = curr_disp_node;
        curr_data_node = curr_data_node->next;
    }

    if(malloc_fail)
    {
        // TODO: Handle the malloc failure by traversing to the beginning of the list
        // TODO: and freeing the data and the nodes

        p_list = NULL;
    }

    // Return the first item in the list
    while (p_list != NULL && p_list->prev != NULL)
    {
        p_list = p_list->prev;
    }

    return p_list;
}

static void gameDisplayObjListDestroy(const gameDisplayNode_t *p_node)
{
    // TODO: implement destroy for list
}

static void gameDisplayGame(SDL_Renderer *renderer, gameDisplayObj_t *game)
{
    assert(game != NULL && renderer != NULL);

    int x = game->pos_x;
    int y = game->pos_y;

    if(game->selected)
    {
        // Display image
        game->thumb.draw(&game->thumb, x, y, SELECTED_IMAGE_SIZE_W, SELECTED_IMAGE_SIZE_H, renderer);

        // Display date above the image
        game->date.draw(&game->date, x, (y - VERTICAL_TEXT_OFFSET), 0, 0, renderer);

        y += SELECTED_IMAGE_SIZE_H;
        // Shift y by image size

        int score_offset = game->score_offset;

        // Offset things in the y direction
        y += (VERTICAL_TEXT_OFFSET);
        game->home_team_name.draw(&game->home_team_name, x, y, 0, 0, renderer);
        game->home_team_score.draw(&game->home_team_score, x + score_offset, y, 0, 0, renderer);
        y += (VERTICAL_TEXT_OFFSET);
        game->away_team_name.draw(&game->away_team_name, x, y, 0, 0, renderer);
        game->away_team_score.draw(&game->away_team_score, x + score_offset, y, 0, 0, renderer);
        y += (VERTICAL_TEXT_OFFSET);
        game->game_state.draw(&game->game_state, x, y, 0, 0, renderer);
    }
    else
    {
        // Display image
        game->thumb.draw(&game->thumb, x, y, UNSELECTED_IMAGE_SIZE_W, UNSELECTED_IMAGE_SIZE_H, renderer);
    }

}


// Finds the selected node in the linked list and returns it
// If NO nodes are selected, then it returns the first one and marks it selected
static const gameDisplayNode_t *gameFindSelectedNode(const gameDisplayNode_t *p_node_list)
{
    // Find selected game, if none are selected, just use the first one
    const gameDisplayNode_t *p_selected_node = p_node_list;
    const gameDisplayNode_t *p_current_node = p_node_list;
    bool found_selected = false;
    while(p_current_node != NULL && !found_selected)
    {
        if(p_current_node->p_data->selected)
        {
            found_selected = true;
            p_selected_node = p_current_node;
        }
        else
        {
            p_current_node = p_current_node->next;
        }
    }

    // Reached the end w/o finding a selected node
    if(!found_selected)
    {
        // In this case, selected node's selected variable isn't actually set, go ahead and set it
        p_selected_node->p_data->selected = true;
    }

    return p_selected_node;
}


static gameDisplayObj_t *gameDisplayObjCreate(const gameData_t *p_game_data)
{
    gameDisplayObj_t *p_game = malloc(sizeof(gameDisplayObj_t));

    if(p_game!= NULL)
    {
        int x = 0;
        int y = 0;
        p_game->pos_x =x;
        p_game->pos_y =y;
        p_game->selected = false;
        p_game->score_offset = (int)(MAX(strlen(p_game_data->home_team_name_str), strlen(p_game_data->home_team_name_str)) * PIX_PER_CHAR);

        p_game->date = textInitObj(p_game_data->date_str, NORMAL_FONT_SIZE, x, y);

        p_game->thumb = imgInitObjBuff(x, y, p_game_data->p_img_data->p_buffer, p_game_data->p_img_data->content_length);
        // Create other text elements
        p_game->game_state = textInitObj(p_game_data->detailed_state_str, NORMAL_FONT_SIZE, x, y);
        p_game->home_team_name = textInitObj(p_game_data->home_team_name_str, NORMAL_FONT_SIZE, x, y);
        p_game->away_team_name = textInitObj(p_game_data->away_team_name_str, NORMAL_FONT_SIZE, x, y);
        p_game->home_team_score = textInitObj(p_game_data->home_team_score_str, NORMAL_FONT_SIZE, x, y);
        p_game->away_team_score = textInitObj(p_game_data->away_team_score_str, NORMAL_FONT_SIZE, x, y);
    }

    return p_game;
}



static gameDisplayObjDestroy(gameDisplayObj_t *p_obj)
{
    free(p_obj);
}
