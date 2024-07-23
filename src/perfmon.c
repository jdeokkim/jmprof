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

#include <fcntl.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <perfmon/pfmlib.h>
#include <perfmon/pfmlib_perf_event.h>

#include "jmprof.h"

/* Private Variables ======================================================> */

static pthread_once_t perfmon_pfm_init_once = PTHREAD_ONCE_INIT;

/* Private Function Prototypes ============================================> */

static void jm_perfmon_pfm_init(void);

static void jm_perfmon_get_event_attr(const char *str,
                                      struct perf_event_attr *attr);

/* Public Functions =======================================================> */

int jm_perfmon_open(void) {
    pthread_once(&perfmon_pfm_init_once, jm_perfmon_pfm_init);

    /*
        NOTE: The following is the list of the event names with unit masks
        that we need for `perf_event_open()`:

        - `MEM_LOAD_RETIRED:L3_MISS:u`
            => "Retired load instructions missed L3 cache as data sources"

        - `MEM_INST_RETIRED:ALL_STORES:u`
            => "All retired store instructions."

        - `MEM_INST_RETIRED:ALL_LOADS:u`
            => "All retired load instructions."

        - `MEM_LOAD_L3_MISS_RETIRED:LOCAL_DRAM:u`
            => "Retired load instructions which data sources missed 
                L3 but serviced from local DRAM."
    */

    struct perf_event_attr hw_event;

    // NOTE: "[pmu_name::]event_name[:unit_mask][:modifier|:modifier=val]"
    jm_perfmon_get_event_attr("MEM_LOAD_RETIRED:L3_MISS:u",
                              &hw_event);

    /*
        NOTE: glibc provides no wrapper for `perf_event_open()`,
        necessitating the use of `syscall()`.
    */

    int fd = syscall(SYS_perf_event_open,
                     &hw_event,            // `struct perf_event_attr *hw_event`
                     0,                    // `pid_t pid`
                     -1,                   // `int cpu`
                     -1,                   // `int group_fd`
                     PERF_FLAG_FD_CLOEXEC  // `unsigned long flags`
    );

    if (fd > 0) {
        (void) ioctl(fd, PERF_EVENT_IOC_RESET, 0);
        (void) ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    }

    return fd;
}

int jm_perfmon_close(int fd) {
    if (fcntl(fd, F_GETFD) < 0) return -1;

    pfm_terminate();

    (void) ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

    return close(fd);
}

/* Private Functions ======================================================> */

static void jm_perfmon_pfm_init(void) {
    (void) pfm_initialize();
}

static void jm_perfmon_get_event_attr(const char *str,
                                      struct perf_event_attr *attr) {
    if (str == NULL || attr == NULL) return;

    attr->type = PERF_TYPE_MAX;

    // NOTE: `hw_event` must be zero-initialized, except for the `size` field
    struct perf_event_attr hw_event = {
        .size = sizeof(hw_event),
        .exclude_kernel = 1,
        .exclude_callchain_kernel = 1,
        .read_format = PERF_FORMAT_TOTAL_TIME_RUNNING
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