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
#include <stdio.h>
#include <string.h>

#include <elfutils/libdwfl.h>

#include "jmprof.h"

/* Typedefs ===============================================================> */

typedef struct jm_inst_t_ {
    uint64_t timestamp;
    signed char opcode;
    uintptr_t ptr;
} jm_inst_t;

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

/* Private Function Prototypes ============================================> */

static void jm_symbols_parse(const char *path);

/* Public Functions =======================================================> */

void jm_symbols_summary(const char *path) {
    jm_symbols_parse(path);

    REENTRANT_PRINTF("SUMMARY: \n");

    {
        /* TODO: ... */
    }

    REENTRANT_PRINTF("\n");
}

/* Private Function Prototypes ============================================> */

static void jm_symbols_parse(const char *path) {
    pthread_mutex_lock(&dwfl_mutex);

    {
        dwfl = dwfl_begin(&dwfl_callbacks);

        FILE *fp = fopen(path, "r");

        char buffer[MAX_BUFFER_SIZE], context[MAX_BUFFER_SIZE];

        while (fgets(buffer, sizeof buffer, fp) != NULL) {
            buffer[strcspn(buffer, "\r\n")] = '\0';

            jm_inst_t inst = { .opcode = JM_OPCODE_UNKNOWN };

            (void) sscanf(buffer,
                          "%" PRIu64 " %c %" PRIuPTR " %s",
                          &inst.timestamp,
                          &inst.opcode,
                          &inst.ptr,
                          context);

            switch (inst.opcode) {
                case JM_OPCODE_ALLOC:
                    /* TODO: ... */

                    break;

                case JM_OPCODE_BACKTRACE:
                    /* TODO: ... */

                    break;

                case JM_OPCODE_EXEC_PATH:
                    break;

                case JM_OPCODE_FREE:
                    /* TODO: ... */

                    break;

                case JM_OPCODE_MODULE:
                    if (strncmp(context,
                                "linux-vdso.so",
                                sizeof "linux-vdso.so")
                        == 0)
                        continue;

                    // TODO: `stderr`
                    (void) dwfl_report_elf(dwfl,
                                           context,
                                           context,
                                           -1,
                                           (GElf_Addr) inst.ptr,
                                           false);

                    break;

                case JM_OPCODE_UPDATE_MODULES:
                    if (context[0] == '<') {
                        dwfl_report_begin(dwfl);
                    } else if (context[0] == '>') {
                        // TODO: `stderr`
                        (void) dwfl_report_end(dwfl, NULL, NULL);
                    }

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

/*
    GElf_Addr elf_addr = (GElf_Addr) inst.ptr;

    // TODO: `stderr`
    Dwfl_Module *mod = dwfl_addrmodule(dwfl, elf_addr);

    GElf_Addr mod_addr_start = 0, mod_addr_end = 0;

    const char *mod_name = dwfl_module_info(
        mod, NULL, &mod_addr_start, &mod_addr_end, NULL, NULL, NULL, NULL);

    GElf_Sym sym_info;

    GElf_Addr sym_offset = 0;

    const char *sym_name = dwfl_module_addrinfo(
        mod, elf_addr, &sym_offset, &sym_info, NULL, NULL, NULL);
*/
