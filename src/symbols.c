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

typedef struct jmBacktrace_ {
    GElf_Addr addr;
    struct jmBacktraceSymbol_ {
        char name[MAX_BUFFER_SIZE];
    } sym;
    struct jmBacktraceModule_ {
        char name[MAX_BUFFER_SIZE];
        GElf_Addr offset;
    } mod;
    struct jmBacktraceSrc_ {
        char name[MAX_BUFFER_SIZE];
        int line, column;
    } src;
} jmBacktrace;

typedef struct jmAllocEntry_ {
    void *key;
    size_t size, bt_top;
    uint64_t timestamp;
    jmBacktrace traces[MAX_BACKTRACE_COUNT];
    UT_hash_handle hh;
} jmAllocEntry;

typedef struct jmSummary_ {
    char exec_path[MAX_BUFFER_SIZE];
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
static jmAllocEntry *jm_symbols_alloc_find_entry(void *key);
static void jm_symbols_alloc_delete_entry(jmAllocEntry *entry);

static void jm_symbols_alloc_push_backtrace(jmAllocEntry *entry,
                                            jmBacktrace bt);

/* ========================================================================> */

static jmBacktrace jm_symbols_build_backtrace(void *ptr);
static jmInst jm_symbols_build_inst(const char *buffer);
static void jm_symbols_parse_log(const char *path);

/* Public Functions =======================================================> */

void jm_symbols_summary(const char *path) {
    jm_symbols_parse_log(path);

    REENTRANT_PRINTF("jmprof v" JMPROF_VERSION " by " JMPROF_AUTHOR "\n\n"
                     "> %s\n\n",
                     summary.exec_path);

    REENTRANT_PRINTF("SUMMARY: \n"
                     "  %d allocs, %d frees (%ld bytes alloc-ed)\n",
                     summary.stats.count[0],
                     summary.stats.count[1],
                     summary.stats.total);

    {
        if (summary.stats.count[0] > 0) REENTRANT_PRINTF("\n");

        jmAllocEntry *head = summary.entries;

        for (; head != NULL; head = head->hh.next) {
            REENTRANT_PRINTF("  ~ alloc #1 (! %" PRIu64
                             " ms) -> [%ld bytes]: \n",
                             head->timestamp,
                             head->size);

            for (int i = 0; i < head->bt_top; i++) {
                jmBacktrace bt = head->traces[i];

                REENTRANT_PRINTF("    @ 0x%jx: %s (%s:%d:%d)\n"
                                 "      (in %s)\n",
                                 bt.addr,
                                 bt.sym.name,
                                 bt.src.name,
                                 bt.src.line,
                                 bt.src.column,
                                 bt.mod.name);
            }

            REENTRANT_PRINTF("\n");
        }

        jmAllocEntry *entry = NULL, *temp = NULL;

        // clang-format off
        HASH_ITER(hh, summary.entries, entry, temp)
            jm_symbols_alloc_delete_entry(entry);
    }

    // REENTRANT_PRINTF("\n");
}

/* Private Function Prototypes ============================================> */

static void jm_symbols_alloc_add_entry(jmInst inst) {
    jmAllocEntry *entry = calloc(1, sizeof(jmAllocEntry));

    entry->key = inst.addr;
    entry->size = malloc_usable_size(inst.addr);
    entry->timestamp = inst.timestamp;

    HASH_ADD_PTR(summary.entries, key, entry);
}

static jmAllocEntry *jm_symbols_alloc_find_entry(void *key) {
    jmAllocEntry *entry = NULL;

    HASH_FIND_PTR(summary.entries, &key, entry);

    return entry;
}

static void jm_symbols_alloc_delete_entry(jmAllocEntry *entry) {
    HASH_DEL(summary.entries, entry);

    free(entry);
}

static void jm_symbols_alloc_push_backtrace(jmAllocEntry *entry,
                                            jmBacktrace bt) {
    entry->traces[entry->bt_top++] = bt;
}

/* ========================================================================> */

static jmBacktrace jm_symbols_build_backtrace(void *ptr) {
    jmBacktrace bt = { .addr = (GElf_Addr) ptr };

    Dwfl_Module *mod = dwfl_addrmodule(dwfl, bt.addr);

    {
        GElf_Addr mod_addr_start = 0, mod_addr_end = 0;

        const char *mod_name = dwfl_module_info(
            mod, NULL, &mod_addr_start, &mod_addr_end, NULL, NULL, NULL, NULL);

        bt.mod.offset = mod_addr_start;

        (void) strcpy(bt.mod.name, mod_name);
    }

    {
        GElf_Sym sym_info;

        GElf_Addr sym_offset = 0;

        const char *sym_name = dwfl_module_addrinfo(
            mod, bt.addr, &sym_offset, &sym_info, NULL, NULL, NULL);

        (void) strcpy(bt.sym.name, sym_name);
    }

    {
        Dwarf_Addr bias = 0;

        Dwarf_Die *die = dwfl_module_addrdie(mod, bt.addr, &bias);

        if (die != NULL) {
            Dwarf_Line *src = dwarf_getsrc_die(die, bt.addr - bias);

            if (src != NULL) {
                const char *src_name = dwarf_linesrc(src, NULL, NULL);

                if (src_name == NULL) src_name = "??";

                (void) strcpy(bt.src.name, src_name);
            }

            (void) dwarf_lineno(src, &bt.src.line);
            (void) dwarf_linecol(src, &bt.src.column);
        }
    }

    return bt;
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

        char buffer[MAX_BUFFER_SIZE];

        void *alloc_ctx = NULL;

        while (fgets(buffer, sizeof buffer, fp) != NULL) {
            jmInst inst = jm_symbols_build_inst(buffer);

            switch (inst.opcode) {
                case JM_OPCODE_ALLOC:
                    jm_symbols_alloc_add_entry(inst);

                    summary.stats.count[0]++;
                    summary.stats.total += malloc_usable_size(inst.addr);

                    alloc_ctx = inst.addr;

                    break;

                case JM_OPCODE_BACKTRACE:
                    assert(alloc_ctx != NULL);

                    jm_symbols_alloc_push_backtrace(jm_symbols_alloc_find_entry(
                                                        alloc_ctx),
                                                    jm_symbols_build_backtrace(
                                                        inst.addr));

                    break;

                case JM_OPCODE_EXEC_PATH:
                    (void) strcpy(summary.exec_path, inst.ctx);

                    break;

                case JM_OPCODE_FREE:
                    summary.stats.count[1]++;
                    summary.stats.total -= malloc_usable_size(inst.addr);

                    alloc_ctx = inst.addr;

                    break;

                case JM_OPCODE_MODULE:
                    if (strncmp(inst.ctx,
                                "linux-vdso.so",
                                sizeof "linux-vdso.so")
                        == 0)
                        continue;

                    (void) dwfl_report_elf(dwfl,
                                           inst.ctx,
                                           inst.ctx,
                                           -1,
                                           (GElf_Addr) inst.addr,
                                           false);

                    break;

                case JM_OPCODE_UPDATE_MODULES:
                    if (inst.ctx[0] == '<') dwfl_report_begin(dwfl);
                    else if (inst.ctx[0] == '>')
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