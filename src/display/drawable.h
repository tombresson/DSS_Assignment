//////////////////////////////////////////////////////////////////////////////
//
//  drawable.h
//
//  Drawable Element
//
//  Contains datastuctures for drawable elements.
//
// The concept around a drawable element is, is a single type that can be
// passed around and drawn. Each sub element that is a drawable element
// implements the draw function, so it can be drawn just by calling the draw
// function contained in the function pointer and passing in the reference to
// itself.
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

#ifndef DRAWABLE_H
#define DRAWABLE_H

/* ***************************    Includes     **************************** */

/* ***************************   Definitions   **************************** */

// Types of drawable elements
// If adding elements that are drawable, add it here, along with the structure below.
typedef enum
{
    E_DRAWABLE_INVALID = 0,
    E_DRAWABLE_TEXT,
    E_DRAWABLE_IMG
} drawableType_t;

typedef enum
{
    E_IMGTYPE_NULL = 0,
    E_IMGTYPE_FILE,
    E_IMGTYPE_BUFF
} imgObjType_t;

typedef struct drawableObj drawableObj_t;

// Function prototype for the draw function
typedef void (drawFcn_t)(drawableObj_t* obj, int x, int y, int w, int h, SDL_Renderer *renderer);

/* ****************************   Structures   **************************** */

// Text drawable element
typedef struct
{
    const char *message;
    int font_size;
    SDL_Texture *texture;
    SDL_Rect rect;
} textObject_t;

// Image objects can be built either from a file name or a data buffer
typedef struct
{
    imgObjType_t type;
    union {
        struct
        {
            const uint8_t *buff;
            size_t buff_len;
        };
        const char *file_name;
    };
    SDL_Texture *texture;
    SDL_Rect rect;
} imgObject_t;


// Drawable element type
struct drawableObj
{
    drawableType_t type;
    union {
        imgObject_t img;
        textObject_t text;
    };
    drawFcn_t *draw;
};

/* ***********************   Function Prototypes   ************************ */

#endif /* DRAWABLE_H */
