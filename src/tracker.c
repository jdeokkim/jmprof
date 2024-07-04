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

/* Includes ===============================================================> */

#define _GNU_SOURCE

#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "jmprof.h"

/* Private Variables ======================================================> */

static pthread_once_t calloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t malloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t realloc_key_once = PTHREAD_ONCE_INIT;

static pthread_once_t free_key_once = PTHREAD_ONCE_INIT;

/* ========================================================================> */

static pthread_once_t dlopen_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t dlclose_key_once = PTHREAD_ONCE_INIT;

/* Private Function Prototypes ============================================> */

// TODO: ...

/* Public Functions =======================================================> */

void jm_tracker_init(void) {
    jm_preload_init();

/* ========================================================================> */

    unsetenv("LD_PRELOAD");

/* ========================================================================> */

    char path[PATH_MAX + 1];

    (void) realpath("/proc/self/exe", path);

    // TODO: ...

/* ========================================================================> */

    atexit(jm_tracker_deinit);
}

/* Private Functions ======================================================> */

void jm_tracker_deinit(void) {
    jm_preload_deinit();

/* ========================================================================> */

    // TODO: ...
}

/* Private Function Prototypes ============================================> */

// TODO: ...
