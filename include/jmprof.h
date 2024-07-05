/*
    Copyright (c) 2024 Jaedeok Kim <jdeokkim@protonmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included 
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

#ifndef JMPROF_H
#define JMPROF_H

/* Includes ===============================================================> */

#include <stdbool.h>

#include "printf.h"

/* Macros =================================================================> */

// clang-format off

#define REENTRANT_PRINTF     printf_
#define REENTRANT_VPRINTF    vprintf_
#define REENTRANT_SPRINTF    sprintf_
#define REENTRANT_VSPRINTF   vsprintf_
#define REENTRANT_SNPRINTF   snprintf_
#define REENTRANT_VSNPRINTF  vsnprintf_

/* ========================================================================> */

#define JM_INIT_ONCE 
#define JM_READ_ONLY 

/* ========================================================================> */

#define MAX_BUFFER_SIZE  8192

// clang-format on

/* Public Function Prototypes =============================================> */

/* (from src/backtrace.c) =================================================> */

void jm_backtrace_unwind(bool is_alloc, const void *ptr);

/* (from src/preload.c) ===================================================> */

JM_INIT_ONCE void jm_preload_init(void);
JM_INIT_ONCE void jm_preload_deinit(void);

/* (from src/symbols.c) ===================================================> */

void jm_symbols_parse(const char *path);

/* (from src/tracker.c) ===================================================> */

JM_INIT_ONCE void jm_tracker_init(void);
JM_INIT_ONCE void jm_tracker_deinit(void);

void jm_tracker_fprintf(const char* format, ...);
void jm_tracker_set_dirty(bool value);
void jm_tracker_update_mappings(void);

#endif // `JMPROF_H`
