//////////////////////////////////////////////////////////////////////////////
//
//  shared_data_types.h
//
//  Shared Data Types
//
//  Set of shared data types used across the app
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

#ifndef SHARED_DATA_TYPES_H
#define SHARED_DATA_TYPES_H

/* ***************************    Includes     **************************** */

/* ***************************   Definitions   **************************** */

// Defines the total length needed to store a ISO8601 string
// the millisecond component may or may not be actively being used
#define ISO8601_TIME_STR_LEN                    (sizeof("2018-12-31T11:59:59.999Z"))

/* ****************************   Structures   **************************** */

typedef struct
{
    size_t size;
    size_t content_length;
    char *p_buffer;
    char *p_pos;
} httpDataBuffer_t;


// Data coming out of objects in the "games" array data
typedef struct
{
    char date_str[ISO8601_TIME_STR_LEN];  // gameDate
    char *home_team_name_str;       // teams.away.team.name
    char *away_team_name_str;       // teams.home.team.name
    char *home_team_score_str;      // teams.away.score
    char *away_team_score_str;      // teams.home.score
    char *detailed_state_str;       // status.detailedState
    httpDataBuffer_t* p_img_data;   // Pointer to data that contains the image data.
}gameData_t;



/* ***********************   Function Prototypes   ************************ */

#endif /* SHARED_DATA_TYPES_H */
