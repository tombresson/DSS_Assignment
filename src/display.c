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
#include "display/game_info.h"
#include "display.h"

/* ***************************   Definitions   **************************** */

#define DISPLAY_BACKGROUND_FILE "res/1.jpg"
#define DISPLAY_LOADING_IMAGE_FILE "res/loading.png"
#define DISPLAY_WINDOW_TITLE        "DSS Assignment"

// Screen dimension constants
#define DISPLAY_SCREEN_WIDTH 1920
#define DISPLAY_SCREEN_HEIGHT 1080

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RMASK = 0xff000000;
#define GMASK = 0x00ff0000;
#define BMASK = 0x0000ff00;
#define AMASK = 0x000000ff;
#else
#define RMASK = 0x000000ff;
#define GMASK = 0x0000ff00;
#define BMASK = 0x00ff0000;
#define AMASK = 0xff000000;
#endif

#define THUMB_H 270
#define THUMB_W 480

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */
static bool init();
static bool loadMedia();
static void close();
static void displayStartDisplay(void);
static void displayShowGameData(void);

SDL_Texture *loadTextureFile(char *file, SDL_Renderer *ren);
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h);

/* ***********************   File Scope Variables   *********************** */

// The window we'll be rendering to
SDL_Window *g_window = NULL;

// Renderer to use
SDL_Renderer *g_renderer = NULL;

// The surface contained by the window
SDL_Surface *g_screen_surface = NULL;

// The image we will load and show on the screen
SDL_Surface *g_background_surface = NULL;
SDL_Surface *g_loading_surface = NULL;

TTF_Font *g_game_display_font = NULL;

/* *************************   Public  Functions   ************************ */

int display()
{
    // Start up SDL and create window
    if (!init())
    {
        printf("Failed to initialize!\n");
    }
    else
    {
        // Load media
        if (!loadMedia())
        {
            printf("Failed to load media!\n");
        }
        else
        {
            displayStartDisplay();
        }
    }

    // Free resources and close SDL
    close();

    return 0;
}

/* *************************   Private Functions   ************************ */

static bool init()
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

static bool loadMedia()
{
    // Loading success flag
    bool success = true;

    // Load splash image
    g_background_surface = IMG_Load(DISPLAY_BACKGROUND_FILE);
    if (g_background_surface == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", DISPLAY_BACKGROUND_FILE, SDL_GetError());
        success = false;
    }

    g_loading_surface = IMG_Load(DISPLAY_LOADING_IMAGE_FILE);
    if (g_loading_surface == NULL)
    {
        printf("Unable to load image %s! SDL Error: %s\n", DISPLAY_LOADING_IMAGE_FILE, SDL_GetError());
        success = false;
    }

    return success;
}

static void close()
{
    // Deallocate surface
    SDL_FreeSurface(g_background_surface);
    g_background_surface = NULL;
    SDL_FreeSurface(g_loading_surface);
    g_loading_surface = NULL;

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

    // Apply the background and the loading images
    SDL_BlitSurface(g_background_surface, NULL, g_screen_surface, NULL);
    SDL_BlitSurface(g_loading_surface, NULL, g_screen_surface, NULL);

    // Update the surface
    SDL_UpdateWindowSurface(g_window);

    // Download the data
    // TODO: make this a separate thread, so the SDL code can continue on and window stays responsive
    //
    // TODO: Take the URL in as a param
    gameDataNode_t *p_game_list =
        gameDataParserGatherData("http://statsapi.mlb.com/api/v1/schedule?hydrate=game(content(editorial(recap))),decisions&date=2018-06-10&sportId=1");

    // Done loading.
    SDL_BlitSurface(g_background_surface, NULL, g_screen_surface, NULL);
    SDL_UpdateWindowSurface(g_window);


    gameDisplayGames()
    SDL_RenderPresent(g_renderer);

    // // Display the game data
    // displayShowGameData();

    // Start polling events
    SDL_Event event;
    bool exit = false;
    while(!exit)
    {
        if(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                exit = true;
                break;

            case SDL_KEYDOWN:
                event.key.keysym

                break;

            default:
                break;
            }
        }
    }
}

static void displayHandleKeyPress(const gameDataNode_t *p_node, const SDL_Keysym key)
{
    switch (key.sym)
    {
        case SDLK_RIGHT:
            break;

        case SDLK_LEFT:
            break;

        case SDLK_q:
            break;

        default:
            break;
    }
}

static void displayShowGameData(void)
{

    // /* SDL interprets each pixel as a 32-bit number, so our masks must depend
    //    on the endianness (byte order) of the machine */
    // SDL_Surface *cut_base_surface = SDL_CreateRGBSurface(0, 480, 270, 32,
    //                                RMASK, GMASK, BMASK, AMASK);

    // SDL_Surface* cut_surface = IMG_Load("cut.jpg");
    // assert(cut_surface != NULL);

    // SDL_Rect srcrect;
    // SDL_Rect dstrect;

    // srcrect.x = 0;
    // srcrect.y = 0;
    // srcrect.w = 32;
    // srcrect.h = 32;
    // dstrect.x = 640/2;
    // dstrect.y = 480/2;
    // dstrect.w = 32;
    // dstrect.h = 32;

    SDL_RenderClear(g_renderer);
    // SDL_RenderDrawRect(g_renderer, &srcrect);
    // SDL_Delay(1000);
    //SDL_Texture* cut_texture = loadTextureFile("cut.jpg", g_renderer);
    SDL_Texture *cut_texture = loadTextureFile("cut.jpg", g_renderer);
    renderTexture(cut_texture, g_renderer, 900, 500, THUMB_W, THUMB_H);
    SDL_RenderPresent(g_renderer);
    SDL_Delay(5000);
}

SDL_Texture *loadTextureFile(char *file, SDL_Renderer *ren)
{
    SDL_Texture *texture = IMG_LoadTexture(ren, file);
    if (texture == NULL)
    {
        printf("LoadTexture Error");
    }
    return texture;
}



/**
* Draw an SDL_Texture to an SDL_Renderer at position x, y, with some desired
* width and height
* @param tex The source texture we want to draw
* @param ren The renderer we want to draw to
* @param x The x coordinate to draw to
* @param y The y coordinate to draw to
* @param w The width of the texture to draw
* @param h The height of the texture to draw
*/
void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h)
{
    //Setup the destination rectangle to be at the position we want
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    dst.w = w;
    dst.h = h;
    SDL_RenderCopy(ren, tex, NULL, &dst);
}
