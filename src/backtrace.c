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

#include <malloc.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "jmprof.h"

/* Private Variables ======================================================> */

static pthread_mutex_t unwind_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Public Functions =======================================================> */

void jm_backtrace_unwind(bool is_alloc, const void *ptr, size_t size) {
    pthread_mutex_lock(&unwind_mutex);

    {
        size = malloc_usable_size((void *) ptr);

        jm_tracker_fprintf("%c 0x%jx %ju\n",
                           (is_alloc ? JM_OPCODE_ALLOC : JM_OPCODE_FREE),
                           (uintptr_t) ptr,
                           size);

        void *traces[MAX_BACKTRACE_COUNT];

        int trace_count = unw_backtrace(traces, MAX_BACKTRACE_COUNT);

        // NOTE: `traces[0] => jm_backtrace_unwind(...)`
        for (int i = 1; i < trace_count; i++)
            jm_tracker_fprintf("b 0x%jx\n", traces[i]);
    }

    pthread_mutex_unlock(&unwind_mutex);
}
