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

#include <stdio.h>

#include <pthread.h>

#include <elfutils/libdwfl.h>

#include "jmprof.h"

/* Constants ==============================================================> */

const Dwfl_Callbacks dwfl_callbacks = {
    .find_elf = dwfl_build_id_find_elf,
    .find_debuginfo = dwfl_standard_find_debuginfo,
    .section_address = dwfl_offline_section_address,
    .debuginfo_path = NULL
};

/* Private Variables ======================================================> */

static pthread_mutex_t dwfl_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ========================================================================> */

static Dwfl *dwfl;

/* Public Functions =======================================================> */

void jm_symbols_parse(const char *path) {
    pthread_mutex_lock(&dwfl_mutex);

    {
        dwfl = dwfl_begin(&dwfl_callbacks);

        FILE *fp = fopen(path, "r");

/* ========================================================================> */

        char buffer[MAX_BUFFER_SIZE];

        // TODO: ...

/* ========================================================================> */

        fclose(fp);

        dwfl_end(dwfl);
    }

    pthread_mutex_unlock(&dwfl_mutex);
}
