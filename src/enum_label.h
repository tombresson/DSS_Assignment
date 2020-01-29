//////////////////////////////////////////////////////////////////////////////
//
//  enum_label.h
//
//  Enum to Label
//
//  Utility that helps conversion of enumerated values to strings and back.
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

#ifndef ENUM_LABEL_H
#define ENUM_LABEL_H

/* ***************************    Includes     **************************** */

#include <stdbool.h>

/* ***************************   Definitions   **************************** */

// Put this at the end of the enumLabel_t array of data to indicate the end
#define ENUM_LABEL_END          { 0, NULL}

/* ****************************   Structures   **************************** */

// !WARNING: An array of this data MUST END WITH and entry of `ENUM_LABEL_END`
typedef struct
{
    int value;
    char *label;
} enumLabel_t;

/* ***********************   Function Prototypes   ************************ */

bool elGetValueFromString(const enumLabel_t *el, const char *const label, int *value, bool case_sensitive);
bool elGetValueFromBuffer(const enumLabel_t *el, const char *const label,
                          int label_size, int *value, bool case_sensitive);
const char *elGetLabel(enumLabel_t const *const el, int value);

#endif /* ENUM_LABEL_H */
