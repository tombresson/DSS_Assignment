//////////////////////////////////////////////////////////////////////////////
//
//  display.c
//
//  Display Module
//
//  Module description in display.h
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
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Library Includes
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

// Project Includes
#include "game_data_parser.h"

// Module Includes
#include "display/text.h"
#include "display/image.h"
#include "display/game_info.h"
#include "display.h"

/* ***************************   Definitions   **************************** */

#define DISPLAY_WINDOW_TITLE "DSS Assignment"

// Resource files
#define DISPLAY_BACKGROUND_FILE "res/1.jpg"
#define DISPLAY_LOADING_IMAGE_FILE "res/loading.png"

// Screen dimension constants
#define DISPLAY_SCREEN_WIDTH 1920
#define DISPLAY_SCREEN_HEIGHT 1080

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */
static bool displayInit(void);
static void displayClose(void);
static void displayStartDisplay(void);
static void displayShowGameData(void);
static void displayHandleKeyPress(const SDL_Keysym key, bool* exit);

/* ***********************   File Scope Variables   *********************** */

// The window we'll be rendering to
SDL_Window *g_window = NULL;

// Renderer to use
SDL_Renderer *g_renderer = NULL;

// The surface contained by the window
SDL_Surface *g_screen_surface = NULL;


/* *************************   Public  Functions   ************************ */

int display(void)
{
    // Start up SDL and create window
    if (!displayInit())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        displayStartDisplay();
    }

    // Free resources and close SDL
    displayClose();


    // TODO: Should return some kind of exit code after leaving
    return 0;
}

/* *************************   Private Functions   ************************ */

static bool displayInit(void)
{
    // Initialization flag
    bool success = true;

    // Initialize SDL
    int main_init_retval = SDL_Init(SDL_INIT_VIDEO);
    if (main_init_retval == -1)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    }

    // load support for the JPG and PNG image formats
    int flags = IMG_INIT_JPG | IMG_INIT_PNG;
    int img_init_retval = IMG_Init(flags);
    if ((img_init_retval & flags) != flags)
    {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
        // handle error
        success = false;
    }

    int ttf_init_retval = TTF_Init();
    if (ttf_init_retval == -1)
    {
        printf("TTF_Init: Failed to init!\n");
        printf("TTF_Init: %s\n", TTF_GetError());
        // handle error
        success = false;
    }

    if (success)
    {
        // Create window
        g_window = SDL_CreateWindow(DISPLAY_WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, DISPLAY_SCREEN_WIDTH, DISPLAY_SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (g_window == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        }
        else
        {
            // Get window surface
            g_screen_surface = SDL_GetWindowSurface(g_window);

            // Create the renderer
            g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (g_renderer == NULL)
            {
                printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            }
        }
    }

    return success;
}

static void displayClose(void)
{
    // Destroy window
    SDL_DestroyRenderer(g_renderer);
    g_window = NULL;

    // Destroy window
    SDL_DestroyWindow(g_window);
    g_window = NULL;

    // Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

static void displayStartDisplay(void)
{
    SDL_RenderClear(g_renderer);
    drawableObj_t background = imgInitObjFile(0, 0, DISPLAY_BACKGROUND_FILE);
    drawableObj_t loading = imgInitObjFile(0, 0, DISPLAY_LOADING_IMAGE_FILE);
    background.draw(&background, 0, 0, 0, 0, g_renderer);
    loading.draw(&loading, 0, 0, 0, 0, g_renderer);
    SDL_RenderPresent(g_renderer);

    // Download the data
    // TODO: make this a separate thread, so the SDL code can continue on and window stays responsive
    //
    // TODO: Take the URL in as a param
    gameDataNode_t *p_game_list =
        gameDataParserGatherData("http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=2018-06-10&sportId=1");

    // Done loading, init the game list and hold onto the event handler
    displayEventHandlerFcn_t *gameDispEvntHandler = gameDisplayInit(p_game_list);

    // Start polling events
    SDL_Event event;
    bool exit = false;
    while (!exit)
    {
        background.draw(&background, 0, 0, 0, 0, g_renderer);
        gameDisplayGames(g_renderer);

        SDL_RenderPresent(g_renderer);

        if (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                exit = true;
                // No longer need the game list, so it can be free'd
                gameDataParserGameListDestroy(p_game_list);
                break;

            case SDL_KEYDOWN:
                displayHandleKeyPress(event.key.keysym, &exit);
                break;

            default:
                break;
            }

            // Call other event handlers
            gameDispEvntHandler(&event);

        }

        SDL_RenderClear(g_renderer);
    }
}


static void displayHandleKeyPress(const SDL_Keysym key, bool *exit)
{
    switch (key.sym)
    {

    case SDLK_q:
        *exit = true;
        break;

    default:
        break;
    }
}
