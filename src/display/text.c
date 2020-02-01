//////////////////////////////////////////////////////////////////////////////
//
//  text.c
//
//  SDL Text Renderer
//
//  Module description in text.h
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
#include <SDL_TTF.h>

// Project Includes

// Module Includes
#include "text.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static SDL_Texture *textGetTexture(SDL_Renderer *renderer, int font_size, char *str, const SDL_Color *color);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

void textDisplay(int x, int y, SDL_Renderer *renderer, textObject_t *text_obj)
{
    text_obj->rect.x = x;
    text_obj->rect.y = y;

    const SDL_Color white = {255,255,255,255};
    SDL_Texture *text_texture = textGetTexture(renderer, text_obj->font_size, text_obj->message, &white);
    // Set the rectangle to the proper height and width, based on the generated texture
    SDL_QueryTexture(text_texture, NULL, NULL, &text_obj->rect.w, &text_obj->rect.h);

    SDL_RenderCopy(renderer, text_texture, NULL, &text_obj->rect);
}

/* *************************   Private Functions   ************************ */

static SDL_Texture *textGetTexture(SDL_Renderer *renderer, int font_size, char *str, const SDL_Color *color)
{
    // Load the font once and only once
    // Does not support muliple fonts
    static TTF_Font *font = NULL;
    if(font == NULL)
    {
        // This can leak if called more than once, and PROBABLY should be cleaned up after
        font = TTF_OpenFont("Myriad Pro Bold.ttf",18);
        if (font == NULL)
        {
            printf("Unable to load font %s! SDL Error: %s\n", "Myriad Pro Bold", TTF_GetError());
        }
    }

    SDL_Texture *text_texture = NULL;
    if(font != NULL)
    {
        SDL_Surface *text_surface  = TTF_RenderText_Solid(font, str, *color);
        if(text_surface != NULL)
        {
            text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
            if(text_texture == NULL)
            {
                printf("Failed to create Text Texture. SDL Error: %s\n", SDL_GetError());
            }

            SDL_FreeSurface(text_surface);
        }
        else
        {
            printf("Failed to create Text Surface. SDL Error: %s\n", SDL_GetError());
        }

    }

    return text_texture;
}