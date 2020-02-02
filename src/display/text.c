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
#include <assert.h>

// Library Includes
#include <SDL.h>
#include <SDL_ttf.h>

// Project Includes

// Module Includes
#include "text.h"

/* ***************************   Definitions   **************************** */

#define FONT_FILE       "res/Myriad Pro Bold.ttf"

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static SDL_Texture *textGetTexture(SDL_Renderer *renderer, int font_size, const char *str, const SDL_Color *color);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

drawableObj_t textInitObj(const char *message, const int font_size, const int x, const int y)
{
    drawableObj_t new_obj =
    {
        // Assign type and functions
        .type = E_DRAWABLE_TEXT,
        .draw = textDisplay,

        .text =
        {
            .message = message,
            .font_size = font_size,
            .rect.x = x,
            .rect.y = y,
            .texture = NULL,
        },
    };

    return new_obj;
}

void textDestroyObj(drawableObj_t *p_text_obj)
{
    assert(p_text_obj->type == E_DRAWABLE_TEXT);
    SDL_DestroyTexture(p_text_obj->text.texture);
}

void textDisplay(drawableObj_t *obj, int x, int y, SDL_Renderer *renderer)
{
    assert(obj->type == E_DRAWABLE_TEXT);

    textObject_t *text_obj = &obj->text;
    text_obj->rect.x = x;
    text_obj->rect.y = y;

    if(text_obj->texture == NULL)
    {
        const SDL_Color white = {255,255,255,255};
        text_obj->texture = textGetTexture(renderer, text_obj->font_size, text_obj->message, &white);
        // Set the rectangle to the proper height and width, based on the generated texture
        SDL_QueryTexture(text_obj->texture, NULL, NULL, &text_obj->rect.w, &text_obj->rect.h);
    }

    SDL_RenderCopy(renderer, text_obj->texture, NULL, &text_obj->rect);
}

/* *************************   Private Functions   ************************ */

static SDL_Texture *textGetTexture(SDL_Renderer *renderer, int font_size, const char *str, const SDL_Color *color)
{
    // Load the font once and only once
    // Does not support multiple fonts
    static TTF_Font *font = NULL;
    if(font == NULL)
    {
        // This can leak if called more than once, and PROBABLY should be cleaned up after
        font = TTF_OpenFont(FONT_FILE,18);
        if (font == NULL)
        {
            printf("Unable to load font %s! SDL Error: %s\n", FONT_FILE, TTF_GetError());
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
