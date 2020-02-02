//////////////////////////////////////////////////////////////////////////////
//
//  text.h
//
//  SDL Text Renderer
//
//  Renders text using SDL
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

#ifndef TEXT_H
#define TEXT_H

/* ***************************    Includes     **************************** */

// Defines the drawable datatypes
#include "drawable.h"

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

drawableObj_t textInitObj(const char *message, const int font_size, const int x, const int y);
void textDestroyObj(drawableObj_t *p_text_obj);
void textDisplay(drawableObj_t *obj, int x, int y, int w, int h, SDL_Renderer *renderer);

#endif /* TEXT_H */
