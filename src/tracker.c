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

#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include <fcntl.h>
#include <libgen.h>
#include <link.h>
#include <unistd.h>

#define SOKOL_TIME_IMPL
#include "sokol_time.h"

#include "jmprof.h"

/* Private Variables ======================================================> */

static pthread_once_t tracker_init_once = PTHREAD_ONCE_INIT;
static pthread_once_t tracker_deinit_once = PTHREAD_ONCE_INIT;

/* ========================================================================> */

static pthread_mutex_t is_dirty_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t is_disabled_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t tracker_fd_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ========================================================================> */

static char exec_path[PATH_MAX + 1];
static char log_path[PATH_MAX + 1];

/* ========================================================================> */

static bool is_dirty = true;
static bool is_disabled = false;

static int tracker_fd = -1;

/* Private Function Prototypes ============================================> */

static void jm_tracker_init_(void);
static void jm_tracker_deinit_(void);

/* ========================================================================> */

static void jm_tracker_atfork_prepare(void);
static void jm_tracker_atfork_parent(void);
static void jm_tracker_atfork_child(void);

/* ========================================================================> */

static int
dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data);

/* Public Functions =======================================================> */

void jm_tracker_init(void) {
    pthread_once(&tracker_init_once, jm_tracker_init_);
}

void jm_tracker_deinit(void) {
    pthread_once(&tracker_deinit_once, jm_tracker_deinit_);
}

void jm_tracker_disable(void) {
    pthread_mutex_lock(&is_disabled_mutex);

    is_disabled = true;

    pthread_mutex_unlock(&is_disabled_mutex);
}

/* ========================================================================> */

void jm_tracker_fprintf(const char *format, ...) {
    pthread_mutex_lock(&tracker_fd_mutex);

    {
        va_list args;

        va_start(args, format);

        uint64_t timestamp = stm_now();

        char buffer[MAX_BUFFER_SIZE];

        // `<TIMESTAMP> <OPERATION> [...]`
        int len = REENTRANT_SNPRINTF(NULL, 0, "%" PRIu64 " ", timestamp);

        /*
            NOTE: The return value of `snprintf()` is the number of bytes 
            that would be written, not including the null terminator.
        */
        (void) REENTRANT_SNPRINTF(buffer, len + 1, "%" PRIu64 " ", timestamp);

        len += REENTRANT_VSNPRINTF(buffer + len,
                                   MAX_BUFFER_SIZE - (len + 1),
                                   format,
                                   args);

        write(tracker_fd, buffer, len);

        va_end(args);
    }

    pthread_mutex_unlock(&tracker_fd_mutex);
}

void jm_tracker_set_dirty(bool value) {
    if (pthread_mutex_trylock(&is_dirty_mutex) != 0) return;

    { is_dirty = value; }

    pthread_mutex_unlock(&is_dirty_mutex);
}

void jm_tracker_update_mappings(void) {
    if (pthread_mutex_trylock(&is_dirty_mutex) != 0) return;

    {
        if (!is_dirty) return;

        is_dirty = false;

        jm_tracker_fprintf("%c 0x%jx <\n", JM_OPCODE_UPDATE_MODULES, NULL);

        dl_iterate_phdr(dl_iterate_phdr_callback, NULL);

        jm_tracker_fprintf("%c 0x%jx >\n", JM_OPCODE_UPDATE_MODULES, NULL);
    }

    pthread_mutex_unlock(&is_dirty_mutex);
}

/* Private Functions ======================================================> */

static void jm_tracker_init_(void) {
    stm_setup();

    jm_preload_init();

    assert(atexit(jm_tracker_deinit) == 0);

    assert(pthread_atfork(jm_tracker_atfork_prepare,
                          jm_tracker_atfork_parent,
                          jm_tracker_atfork_child)
           == 0);

    if (readlink("/proc/self/exe", exec_path, PATH_MAX) == -1)
        REENTRANT_SNPRINTF(exec_path, sizeof "unknown", "unknown");

    int len = REENTRANT_SNPRINTF(NULL,
                                 0,
                                 "/tmp/jmprof.%s.%jd",
                                 basename(exec_path),
                                 (intmax_t) getpid());

    (void) REENTRANT_SNPRINTF(log_path,
                              len,
                              "/tmp/jmprof.%s.%jd",
                              basename(exec_path),
                              (intmax_t) getpid());

    tracker_fd = open(log_path, O_CLOEXEC | O_CREAT | O_WRONLY, (mode_t) 0644);

    jm_tracker_fprintf("%c 0x%jx %s\n", JM_OPCODE_EXEC_PATH, NULL, exec_path);
}

static void jm_tracker_deinit_(void) {
    jm_preload_deinit();

    if (close(tracker_fd) == 0) {
        REENTRANT_PRINTF(SEPARATOR_MESSAGE);

        jm_symbols_summary(log_path);
    }
}

/* ========================================================================> */

static void jm_tracker_atfork_prepare(void) {
    // TODO: ...
}

static void jm_tracker_atfork_parent(void) {
    // TODO: ...
}

static void jm_tracker_atfork_child(void) {
    // TODO: ...
}

/* ========================================================================> */

static int
dl_iterate_phdr_callback(struct dl_phdr_info *info, size_t size, void *data) {
    const char *dlpi_name = info->dlpi_name;

    if (dlpi_name == NULL || !dlpi_name[0]) dlpi_name = exec_path;

    jm_tracker_fprintf("%c 0x%jx %s\n",
                       JM_OPCODE_MODULE,
                       info->dlpi_addr,
                       dlpi_name);

    return 0;
}
