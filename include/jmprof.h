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

#define _GNU_SOURCE

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>

#include "printf.h"

/* Macros =================================================================> */

/* clang-format off */

#define JMPROF_AUTHOR        "Jaedeok Kim (jdeokkim@protonmail.com)"
#define JMPROF_VERSION       "0.0.3"

/* ========================================================================> */

#define REENTRANT_PRINTF     printf_
#define REENTRANT_VPRINTF    vprintf_
#define REENTRANT_SPRINTF    sprintf_
#define REENTRANT_VSPRINTF   vsprintf_
#define REENTRANT_SNPRINTF   snprintf_
#define REENTRANT_VSNPRINTF  vsnprintf_

/* ========================================================================> */

#define MAX_BACKTRACE_COUNT  32
#define MAX_BUFFER_SIZE      2048

/* clang-format on */

/* Typedefs ===============================================================> */

typedef enum jmOpcode {
    JM_OPCODE_UNKNOWN,
    JM_OPCODE_ALLOC          = 'a',
    JM_OPCODE_BACKTRACE      = 'b',
    JM_OPCODE_FREE           = 'f',
    JM_OPCODE_MODULE         = 'm',
    JM_OPCODE_UPDATE_MODULES = 'u',
    JM_OPCODE_EXEC_PATH      = 'x'
} jmOpcode;

/* Public Function Prototypes =============================================> */

/* (from src/backtrace.c) =================================================> */

void jm_backtrace_unwind(bool is_alloc, const void *ptr, size_t size);

/* (from src/preload.c) ===================================================> */

void jm_preload_init(void);
void jm_preload_deinit(void);

/* (from src/tracker.c) ===================================================> */

void jm_tracker_init(void);
void jm_tracker_deinit(void);

void jm_tracker_fprintf(const char* format, ...);
void jm_tracker_set_dirty(bool value);
void jm_tracker_update_mappings(void);

/* ========================================================================> */

#endif // `JMPROF_H`
