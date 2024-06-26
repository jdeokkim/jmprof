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

#include <assert.h>
#include <stddef.h>
#include <dlfcn.h>
#include <unistd.h>

/* Typedefs ===============================================================> */

typedef void *(libc_malloc_t)(size_t);

/* Private Variables ======================================================> */

static libc_malloc_t *libc_malloc;

/* Public Functions =======================================================> */

void *malloc(size_t size) {
    void *libc_malloc_ptr = dlsym(RTLD_NEXT, "malloc");

    libc_malloc = libc_malloc_ptr;

    assert(libc_malloc != NULL);

    {
        // TODO: ...
        write(
            STDERR_FILENO, 
            "write(): intercepted malloc()\n", 
            sizeof "write(): intercepted malloc()\n"
        );
    }

    return libc_malloc(size);
}
