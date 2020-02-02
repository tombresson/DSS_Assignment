//////////////////////////////////////////////////////////////////////////////
//
//  display_types.h
//
//  Display Module Datatypes
//
//  Contains common datatypes used in the display subsyste,
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

#ifndef DISPLAY_TYPES_H
#define DISPLAY_TYPES_H

/* ***************************    Includes     **************************** */

// Include SDL for the SDL datatype
#include <SDL.h>

/* ***************************   Definitions   **************************** */

// Event Handler function - For modules to implement their own event handlers
typedef void (displayEventHandlerFcn_t)(const SDL_Event *p_event);

/* ****************************   Structures   **************************** */

/* ***********************   Function Prototypes   ************************ */

#endif /* DISPLAY_TYPES_H */
