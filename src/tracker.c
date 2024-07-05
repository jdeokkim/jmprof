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
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <libgen.h>
#include <pthread.h>
#include <unistd.h>

#include "jmprof.h"

/* Macros =================================================================> */

// clang-format off

#define MAX_BUFFER_SIZE  8192

// clang-format on

/* Private Variables ======================================================> */

static pthread_once_t tracker_init_once = PTHREAD_ONCE_INIT;

/* ========================================================================> */

static pthread_mutex_t tracker_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ========================================================================> */

static int tracker_file_fd = -1;

/* Private Function Prototypes ============================================> */

static void jm_tracker_init_(void);
static void jm_tracker_deinit_(void);

/* Public Functions =======================================================> */

JM_INIT_ONCE void jm_tracker_init(void) {
    pthread_once(&tracker_init_once, jm_tracker_init_);
}

JM_INIT_ONCE void jm_tracker_deinit(void) {
    pthread_once(&tracker_init_once, jm_tracker_deinit_);
}

void jm_tracker_fprintf(const char* format, ...) {
    pthread_mutex_lock(&tracker_file_mutex);

    {
        va_list args;

        va_start(args, format);

        char buffer[MAX_BUFFER_SIZE];

        // `<TIMESTAMP> <OPERATION> [...]`
        int len = REENTRANT_SNPRINTF(
            NULL, 0, "%u ", (unsigned) time(NULL)
        );

        /*
            NOTE: The return value of `snprintf()` is the number of bytes 
            that would be written, not including the null terminator.
        */
        (void) REENTRANT_SNPRINTF(
            buffer, len + 1, "%u ", (unsigned) time(NULL)
        );

        len += REENTRANT_VSNPRINTF(
            buffer + len, MAX_BUFFER_SIZE, format, args
        );

        write(tracker_file_fd, buffer, len);

        va_end(args);
    }

    pthread_mutex_unlock(&tracker_file_mutex);
}

/* Private Functions ======================================================> */

static void jm_tracker_init_(void) {
    jm_preload_init();

/* ========================================================================> */

    unsetenv("LD_PRELOAD");

/* ========================================================================> */

    char path_[PATH_MAX + 1];

    if (readlink("/proc/self/exe", path_, PATH_MAX) == -1)
        REENTRANT_SNPRINTF(path_, sizeof "unknown", "unknown");

    int len = REENTRANT_SNPRINTF(
        NULL, 0, "jmprof.%s.%jd", basename(path_), (intmax_t) getpid()
    );

    char path[PATH_MAX + 1];

    (void) REENTRANT_SNPRINTF(
        path, len, "jmprof.%s.%jd", basename(path_), (intmax_t) getpid()
    );

    tracker_file_fd = open(
        path, 
        O_CLOEXEC | O_CREAT | O_WRONLY, 
        (mode_t) 0644
    );

/* ========================================================================> */

    jm_tracker_fprintf("x %s\n", path_);

/* ========================================================================> */

    atexit(jm_tracker_deinit);  
}

static void jm_tracker_deinit_(void) {
    jm_preload_deinit();

/* ========================================================================> */

    close(tracker_file_fd);
}