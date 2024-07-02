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

#include <pthread.h>

#include "jmprof.h"

/* Private Variables ======================================================> */

static pthread_once_t calloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t malloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t realloc_key_once = PTHREAD_ONCE_INIT;

static pthread_once_t free_key_once = PTHREAD_ONCE_INIT;

/* Public Functions =======================================================> */

void jm_initialize(void) {
    (void) pthread_once(&calloc_key_once, calloc_init_once);
    (void) pthread_once(&malloc_key_once, malloc_init_once);
    (void) pthread_once(&realloc_key_once, realloc_init_once);
    
    (void) pthread_once(&free_key_once, free_init_once);
}
