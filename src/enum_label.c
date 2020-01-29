//////////////////////////////////////////////////////////////////////////////
//
//  enum_label.c
//
//  Enum to Label
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
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

// Module Includes
#include "enum_label.h"

/* *************************   Public  Functions   ************************ */

//
// Figures out what label is associated with a value from a {value, label} table
//
const char *elGetLabel(enumLabel_t const *el, const int value)
{
    assert(el != NULL);
    char *label = NULL;

    while ((NULL == label) && (el->label != NULL))
    {
        if (el->value == value)
        {
            label = el->label;
        }
        else
        {
            ++el;
        }
    }

    return label;
}

//
// Figures out what value is associated with a value from a {value, label} table.
//
bool elGetValueFromString(const enumLabel_t *el, const char *const label, int *value, bool case_sensitive)
{
    bool found = false;

    while (!found && (el->label != NULL))
    {
        int cmp_res;

        if (case_sensitive)
        {
            cmp_res = strcmp(el->label, label);
        }
        else
        {
            cmp_res = strcasecmp(el->label, label);
        }

        if (cmp_res == 0)
        {
            *value = el->value;
            found = true;
        }
        else
        {
            ++el;
        }
    }

    return found;
}

//
// Figures out what value is associated with a value from a {value, label} table passing in a
// character buffer for the label, not NULL terminated.
//
bool elGetValueFromBuffer(const enumLabel_t *el, const char *const label,
                          int label_size, int *value, bool case_sensitive)
{
    bool found = false;

    while (!found && (el->label != NULL))
    {
        int cmp_res;

        if (case_sensitive)
        {
            cmp_res = strncmp(el->label, label, label_size);
        }
        else
        {
            cmp_res = strncasecmp(el->label, label, label_size);
        }

        if (cmp_res == 0)
        {
            *value = el->value;
            found = true;
        }
        else
        {
            ++el;
        }
    }

    return found;
}
