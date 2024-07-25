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

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>

#include "jmprof.h"

/* Typedefs ===============================================================> */

typedef struct jmEventCounter_ {
    int fd;
    uint64_t id;
    char name[MAX_BUFFER_SIZE];
} jmEventCounter;

/*
    NOTE: If a struct defines at least one named member, it is allowed to 
    additionally declare its last member with incomplete array type. 
    
    When an element of the flexible array member is accessed (in an 
    expression that uses operator `.` or `->` with the flexible array member's
    name as the right-hand-side operand), then the struct behaves as if 
    the array member had the longest size fitting in the memory allocated 
    for this object. (since C99)

    - https://en.cppreference.com/w/c/language/struct
*/

typedef struct jmEventReadFormat_ {
    uint64_t event_count;
    uint64_t time_running;
    struct {
        uint64_t value, id;
    } events[];
} jmEventReadFormat;

/* Private Variables ======================================================> */

static jmEventCounter counters[] = {
    { .fd = -1, .name = "LLC_MISSES:u" },
    { .fd = -1, .name = "MEM_LOAD_RETIRED:L3_MISS:u" },
};

/* Private Function Prototypes ============================================> */

static bool jm_perfmon_init(void);
static bool jm_perfmon_deinit(void);

static void jm_perfmon_read_events(void);

static void jm_perfmon_get_event_attr(const char *str,
                                      struct perf_event_attr *attr);

/* Public Functions =======================================================> */

int main(int argc, char *argv[]) {
    if (!jm_perfmon_init()) {
        char buffer[MAX_BUFFER_SIZE] = "";

        (void) strncpy(buffer, strerror(errno), MAX_BUFFER_SIZE - 1);

        for (char *c = buffer; *c != '\0' && (*c = tolower(*c)); ++c)
            ;

        fprintf(stderr,
                "%s: error: jm_perfmon_init(): %s\n",
                argv[0],
                buffer);

        return 1;
    }

    {
        // TODO: ...
    }

    (void) jm_perfmon_deinit();

    return 0;
}

/* Private Functions ======================================================> */

static bool jm_perfmon_init(void) {
    (void) pfm_initialize();

    for (int i = 0, j = (sizeof counters / sizeof *counters); i < j; i++) {
        struct perf_event_attr hw_event;

        // NOTE: "[pmu_name::]event_name[:unit_mask][:modifier|:modifier=val]"
        jm_perfmon_get_event_attr(counters[i].name, &hw_event);

        /*
            NOTE: glibc provides no wrapper for `perf_event_open()`,
            necessitating the use of `syscall()`.
        */

        int group_fd = (i > 0) ? counters[0].fd : -1;

        counters[i]
            .fd = syscall(SYS_perf_event_open,
                          &hw_event,  // `struct perf_event_attr *hw_event`
                          0,          // `pid_t pid`
                          -1,         // `int cpu`
                          group_fd,   // `int group_fd`
                          PERF_FLAG_FD_CLOEXEC  // `unsigned long flags`
        );

        if (counters[i].fd < 0) return false;

        (void) ioctl(counters[i].fd, PERF_EVENT_IOC_ID, &counters[i].id);
    }

    /* clang-format off */

    (void) ioctl(counters[0].fd, 
                    PERF_EVENT_IOC_RESET, 
                    PERF_IOC_FLAG_GROUP);

    (void) ioctl(counters[0].fd,
                    PERF_EVENT_IOC_ENABLE,
                    PERF_IOC_FLAG_GROUP);

    /* clang-format on */

    return true;
}

static bool jm_perfmon_deinit(void) {
    pfm_terminate();

    for (int i = 0, j = (sizeof counters / sizeof *counters); i < j; i++) {
        if (fcntl(counters[i].fd, F_GETFD) < 0) continue;

        (void) ioctl(counters[i].fd,
                     PERF_EVENT_IOC_DISABLE,
                     PERF_IOC_FLAG_GROUP);

        if (close(counters[i].fd) < 0) return false;
    }

    return true;
}

static void jm_perfmon_read_events(void) {
    /*
        NOTE: Events come in two flavors: counting and sampled. 
        
        A counting event is one that is used for counting the aggregate 
        number of events that occur. In general, counting event results are
        gathered with a `read()` call.  
        
        A sampling event periodically writes measurements to a buffer 
        that can then be accessed via `mmap()`.
    */

    // TODO: ...
}

static void jm_perfmon_get_event_attr(const char *str,
                                      struct perf_event_attr *attr) {
    if (str == NULL || attr == NULL) return;

    attr->type = PERF_TYPE_MAX;

    // NOTE: `hw_event` must be zero-initialized, except for the `size` field
    struct perf_event_attr hw_event = {
        .size = sizeof(hw_event),
        .sample_type = PERF_SAMPLE_IP | PERF_SAMPLE_TIME | PERF_SAMPLE_ADDR
                       | PERF_SAMPLE_WEIGHT | PERF_SAMPLE_PHYS_ADDR,
        .read_format = PERF_FORMAT_ID | PERF_FORMAT_GROUP,
        .exclude_hv = 1,
        .exclude_kernel = 1
    };

    pfm_perf_encode_arg_t arg = { .attr = &hw_event,
                                  .size = sizeof(pfm_perf_encode_arg_t) };

    /*
        NOTE: `PFM_PLM0` is not needed since we only need to 
        inspect user-space (unprivileged, ring 3) events
    */

    pfm_err_t ret = pfm_get_os_event_encoding(str,
                                              PFM_PLM3,
                                              PFM_OS_PERF_EVENT_EXT,
                                              &arg);

    if (ret != PFM_SUCCESS) return;

    *attr = hw_event;
}