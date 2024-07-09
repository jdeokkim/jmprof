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
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <elfutils/libdwfl.h>

#define HASH_DEBUG
#include "uthash.h"

#include "jmprof.h"

/* Typedefs ===============================================================> */

typedef struct jmInst_ {
    char opcode, ctx[MAX_BUFFER_SIZE];
    uint64_t timestamp;
    void *addr;
} jmInst;

typedef struct jmAllocEntry_ {
    void *key;
    size_t size;
    uint64_t timestamp;
    UT_hash_handle hh;
} jmAllocEntry;

typedef struct jmSummary_ {
    struct jmSummaryStats_ {
        int count[2];
        size_t total;
    } stats;
    jmAllocEntry *entries;
} jmSummary;

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

/* ========================================================================> */

static jmSummary summary;

/* Private Function Prototypes ============================================> */

static void jm_symbols_alloc_add_entry(jmInst inst);
static void jm_symbols_alloc_delete_entry(jmAllocEntry *entry);

static jmInst jm_symbols_build_inst(const char *buffer);
static void jm_symbols_parse_log(const char *path);

/* Public Functions =======================================================> */

void jm_symbols_summary(const char *path) {
    jm_symbols_parse_log(path);

    REENTRANT_PRINTF("SUMMARY: \n");

    REENTRANT_PRINTF("  %d allocs, %d frees (%ld bytes alloc-ed)\n",
                     summary.stats.count[0],
                     summary.stats.count[1],
                     summary.stats.total);

    {
        if (summary.stats.count[0] > 0) REENTRANT_PRINTF("\n");

        jmAllocEntry *head = summary.entries;

        for (; head != NULL; head = head->hh.next) {
            REENTRANT_PRINTF("  ~ alloc #1 (@ %" PRIu64
                             " ms) -> [%ld bytes]: \n",
                             head->timestamp,
                             head->size);

            /* TODO: ... */
        }

        jmAllocEntry *entry = NULL, *temp = NULL;

        HASH_ITER(hh, summary.entries, entry, temp)
            jm_symbols_alloc_delete_entry(entry);
    }

    REENTRANT_PRINTF("\n");
}

/* Private Function Prototypes ============================================> */

static void jm_symbols_alloc_add_entry(jmInst inst) {
    jmAllocEntry *entry = calloc(1, sizeof(jmAllocEntry));

    entry->key = inst.addr;
    entry->size = malloc_usable_size(inst.addr);
    entry->timestamp = inst.timestamp;

    HASH_ADD_PTR(summary.entries, key, entry);
}

static void jm_symbols_alloc_delete_entry(jmAllocEntry *entry) {
    HASH_DEL(summary.entries, entry);

    free(entry);
}

static jmInst jm_symbols_build_inst(const char *buffer) {
    jmInst inst = { .opcode = JM_OPCODE_UNKNOWN };

    (void) sscanf(buffer,
                  "%" PRIu64 " %c %p %s",
                  &inst.timestamp,
                  &inst.opcode,
                  &inst.addr,
                  inst.ctx);

    inst.ctx[strcspn(inst.ctx, "\r\n")] = '\0';

    return inst;
}

static void jm_symbols_parse_log(const char *path) {
    pthread_mutex_lock(&dwfl_mutex);

    {
        dwfl = dwfl_begin(&dwfl_callbacks);

        FILE *fp = fopen(path, "r");

        char buffer[MAX_BUFFER_SIZE], context[MAX_BUFFER_SIZE];

        while (fgets(buffer, sizeof buffer, fp) != NULL) {
            jmInst inst = jm_symbols_build_inst(buffer);

            switch (inst.opcode) {
                case JM_OPCODE_ALLOC:
                    summary.stats.count[0]++;
                    summary.stats.total += malloc_usable_size(inst.addr);

                    jm_symbols_alloc_add_entry(inst);

                    break;

                case JM_OPCODE_BACKTRACE:
                    /* TODO: ... */

                    break;

                case JM_OPCODE_EXEC_PATH:
                    break;

                case JM_OPCODE_FREE:
                    summary.stats.count[1]++;
                    summary.stats.total -= malloc_usable_size(inst.addr);

                    break;

                case JM_OPCODE_MODULE:
                    if (strncmp(context,
                                "linux-vdso.so",
                                sizeof "linux-vdso.so")
                        == 0)
                        continue;

                    (void) dwfl_report_elf(dwfl,
                                           context,
                                           context,
                                           -1,
                                           (GElf_Addr) inst.addr,
                                           false);

                    break;

                case JM_OPCODE_UPDATE_MODULES:
                    if (context[0] == '<') dwfl_report_begin(dwfl);
                    else if (context[0] == '>')
                        (void) dwfl_report_end(dwfl, NULL, NULL);

                    break;

                default:
                    inst.opcode = JM_OPCODE_UNKNOWN;

                    break;
            }
        }

        fclose(fp);

        dwfl_end(dwfl);
    }

    pthread_mutex_unlock(&dwfl_mutex);
}