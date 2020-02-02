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
#include <assert.h>

// Library Includes
#include <SDL.h>
#include <SDL_image.h>

// Project Includes

// Module Includes
#include "image.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

static SDL_Texture *imgGetTextureFromImgData(SDL_Renderer *renderer, const uint8_t *buff, const size_t buff_len);
static SDL_Texture *imgGetTextureFromImgFile(SDL_Renderer *renderer, const char *file_name);

/* ***********************   File Scope Variables   *********************** */

/* *************************   Public  Functions   ************************ */

drawableObj_t imgInitObjBuff(const int x, const int y, const uint8_t *buff, const size_t buff_len)
{
    drawableObj_t new_obj =
    {
        // Assign type and functions
        .type = E_DRAWABLE_IMG,
        .draw = imgDisplay,

        .img =
        {
            .type = E_IMGTYPE_BUFF,
            .rect.x = x,
            .rect.y = y,
            .texture = NULL,
            .buff = buff,
            .buff_len = buff_len
        }
    };

    return new_obj;
}

drawableObj_t imgInitObjFile(const int x, const int y, const char *file_name)
{
    drawableObj_t new_obj =
    {
        // Assign type and functions
        .type = E_DRAWABLE_IMG,
        .draw = imgDisplay,

        .img =
        {
            .type = E_IMGTYPE_FILE,
            .rect.x = x,
            .rect.y = y,
            .texture = NULL,
            .file_name = file_name
        }
    };

    return new_obj;
}

void imgDestroyObj(drawableObj_t *p_img_obj)
{
    assert(p_img_obj->type == E_DRAWABLE_IMG);
    SDL_DestroyTexture(p_img_obj->img.texture);
}

void imgDisplay(drawableObj_t *obj, int x, int y, int w, int h, SDL_Renderer *renderer)
{
    assert(obj->type == E_DRAWABLE_IMG);

    imgObject_t *img_obj = &obj->img;
    img_obj->rect.x = x;
    img_obj->rect.y = y;

    // Width and height are options
    if(w > 0 && h > 0)
    {
        img_obj->rect.w = w;
        img_obj->rect.h = h;
    }

    if (img_obj->texture == NULL)
    {
        switch (img_obj->type)
        {
        case E_IMGTYPE_BUFF:
            img_obj->texture = imgGetTextureFromImgData(renderer, img_obj->buff, img_obj->buff_len);
            break;

        case E_IMGTYPE_FILE:
            img_obj->texture = imgGetTextureFromImgFile(renderer, img_obj->file_name);
            break;

        default:
            // Type not supported
            assert(false);
            break;
        }
        SDL_QueryTexture(img_obj->texture, NULL, NULL, &img_obj->rect.w, &img_obj->rect.h);
    }

    SDL_RenderCopy(renderer, img_obj->texture, NULL, &img_obj->rect);
}

/* *************************   Private Functions   ************************ */

static SDL_Texture *imgGetTextureFromImgFile(SDL_Renderer *renderer, const char *file_name)
{
    return IMG_LoadTexture(renderer, file_name);
}

static SDL_Texture *imgGetTextureFromImgData(SDL_Renderer *renderer, const uint8_t *buff, const size_t buff_len)
{
    // Create surface from data in buffer
    return IMG_LoadTexture_RW(renderer, SDL_RWFromMem((void *)buff, (int)buff_len), 1);
}
