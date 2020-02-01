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

/* ***************************   Definitions   **************************** */

/* ****************************   Structures   **************************** */

typedef struct
{
    const char *message;
    int font_size;
    SDL_Rect rect;
}textObject_t;


/* ***********************   Function Prototypes   ************************ */

textObject_t textCreateObj(const char *message, const int font_size, const int x, const int y);
void textDisplay(int x, int y, SDL_Renderer *renderer, textObject_t *text_obj);

#endif /* TEXT_H */
