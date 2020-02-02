//////////////////////////////////////////////////////////////////////////////
//
//  image.c
//
//  SDL Image Renderer
//
//  Module description in image.h
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
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// Library Includes
#include <SDL.h>
#include <SDL_image.h>

// Project Includes

// Module Includes
#include "image.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static SDL_Texture *imgGetTexture(SDL_Renderer *renderer, int font_size, char *str, const SDL_Color *color);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

imgObject_t imgInitObj(const int x, const int y, const uint8_t *buff, const size_t buff_len)
{
    imgObject_t new_img_obj =
    {
        .rect.x = x,
        .rect.y = y,
        .texture = NULL,
        .buff = buff,
        .buff_len = buff_len
    };

    return new_img_obj;
}

void imgDestroyObj(imgObject_t *p_img_obj)
{
    SDL_DestroyTexture(p_img_obj->texture);
}

void imgDisplay(int x, int y, SDL_Renderer *renderer, imgObject_t *img_obj)
{
    img_obj->rect.x = x;
    img_obj->rect.y = y;

    if(img_obj->texture == NULL)
    {
        img_obj->texture = imgGetTexture(renderer, img_obj->buff, img_obj->buff_len);
    }

    SDL_RenderCopy(renderer, img_obj->texture, NULL, &img_obj->rect);
}

/* *************************   Private Functions   ************************ */

static SDL_Texture *imgGetTextureFromImgData(SDL_Renderer *renderer, const uint8_t *buff, const size_t buff_len)
{
    // Create surface from data in buffer
    return IMG_LoadTexture_RW(renderer, SDL_RWFromMem(buff,buff_len), 1);
}
