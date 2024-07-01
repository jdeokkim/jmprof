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

#include "jmprof.h"

/* Typedefs ===============================================================> */

// typedef void *(jm_libc_aligned_alloc_t)(size_t alignment, size_t size);
typedef void *(jm_libc_calloc_t)(size_t num, size_t size);
typedef void *(jm_libc_malloc_t)(size_t size);
typedef void *(jm_libc_realloc_t)(void *ptr, size_t new_size);

typedef void (jm_libc_free_t)(void *ptr);

/* typedef void *(jm_posix_mmap_t)(void *addr, size_t len, int prot, 
    int flags, int fildes, off_t off); */
typedef void *(jm_posix_sbrk_t)(intptr_t incr);

/* Private Variables ======================================================> */

// static jm_libc_aligned_alloc_t *libc_aligned_alloc;
static jm_libc_calloc_t *libc_calloc;
static jm_libc_malloc_t *libc_malloc;
static jm_libc_realloc_t *libc_realloc;

static jm_libc_free_t *libc_free;

// static jm_posix_mmap_t *posix_mmap;
static jm_posix_sbrk_t *posix_sbrk;

/* Public Functions =======================================================> */

void *calloc(size_t num, size_t size) {
#ifdef __GLIBC__
    extern void *__libc_calloc(size_t num, size_t size);

    void *libc_calloc_ptr = __libc_calloc;
#else
    void *libc_calloc_ptr = dlsym(RTLD_NEXT, "calloc");
#endif

    if (libc_calloc == NULL)
        libc_calloc = libc_calloc_ptr;

    assert(libc_calloc != NULL);

    void *result = libc_calloc(num, size);

    {
        // TODO: ...
        REENTRANT_PRINTF(
            "jmprof: intercepted %s() at %p\n", 
            __func__, result
        );
    }

    return result;
}

void *malloc(size_t size) {
    void *libc_malloc_ptr = dlsym(RTLD_NEXT, "malloc");

    if (libc_malloc == NULL)
        libc_malloc = libc_malloc_ptr;

    assert(libc_malloc != NULL);

    void *result = libc_malloc(size);

    {
        // TODO: ...
        REENTRANT_PRINTF(
            "jmprof: intercepted %s() at %p\n", 
            __func__, result
        );

        // jm_print_backtrace();
    }

    return result;
}

void *realloc(void *ptr, size_t new_size) {
    void *libc_realloc_ptr = dlsym(RTLD_NEXT, "realloc");

    if (libc_realloc == NULL)
        libc_realloc = libc_realloc_ptr;

    assert(libc_realloc != NULL);

    void *result = libc_realloc(ptr, new_size);

    {
        // TODO: ...
        REENTRANT_PRINTF(
            "jmprof: intercepted %s() at %p\n", 
            __func__, result
        );
    }

    return result;
}

void free(void *ptr) {
    void *libc_free_ptr = dlsym(RTLD_NEXT, "free");

    if (libc_free == NULL)
        libc_free = libc_free_ptr;

    assert(libc_free != NULL);

    return libc_free(ptr);
}

/* ========================================================================> */

void *sbrk(intptr_t incr) {
    void *posix_sbrk_ptr = dlsym(RTLD_NEXT, "sbrk");

    if (posix_sbrk == NULL)
        posix_sbrk = posix_sbrk_ptr;

    assert(posix_sbrk != NULL);

    {
        // TODO: ...
    }

    return posix_sbrk(incr);
}

/* ========================================================================> */

PRINTF_VISIBILITY void putchar_(char c) {
    write(1, &c, 1);
}
