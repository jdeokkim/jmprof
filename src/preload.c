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
#include <pthread.h>
#include <unistd.h>

#include "jmprof.h"

/* Typedefs ===============================================================> */

typedef void *(jm_libc_calloc_t)(size_t num, size_t size);
typedef void *(jm_libc_malloc_t)(size_t size);
typedef void *(jm_libc_realloc_t)(void *ptr, size_t new_size);

typedef void (jm_libc_free_t)(void *ptr);

/* Private Variables ======================================================> */

static jm_libc_calloc_t *libc_calloc;
static jm_libc_malloc_t *libc_malloc;
static jm_libc_realloc_t *libc_realloc;

static jm_libc_free_t *libc_free;

/* ========================================================================> */

static pthread_key_t calloc_key;
static pthread_key_t malloc_key;
static pthread_key_t realloc_key;

static pthread_key_t free_key;

/* Public Functions =======================================================> */

void *calloc(size_t num, size_t size) {
    if (libc_calloc == NULL) jm_initialize();

    void *result = libc_calloc(num, size);

    if ((pthread_getspecific(calloc_key)) == NULL) {
        pthread_setspecific(calloc_key, &calloc_key);

        // TODO: ...
        REENTRANT_PRINTF(
            "jmprof: intercepted %s() at %p\n", 
            __func__, result
        );

        pthread_setspecific(calloc_key, NULL);
    }

    return result;
}

void *malloc(size_t size) {
    if (libc_malloc == NULL) jm_initialize();

    void *result = libc_malloc(size);

    if ((pthread_getspecific(malloc_key)) == NULL) {
        pthread_setspecific(malloc_key, &malloc_key);

        // TODO: ...
        REENTRANT_PRINTF(
            "jmprof: intercepted %s() at %p\n", 
            __func__, result
        );

        jm_print_backtrace();

        pthread_setspecific(malloc_key, NULL);
    }

    return result;
}

void *realloc(void *ptr, size_t new_size) {
    if (libc_realloc == NULL) jm_initialize();

    void *result = libc_realloc(ptr, new_size);

    if ((pthread_getspecific(realloc_key)) == NULL) {
        pthread_setspecific(realloc_key, &realloc_key);

        if (ptr == NULL) {
            // TODO: `malloc()`
        } else if (new_size == 0) {
            // TODO: `free()`
        }

        pthread_setspecific(realloc_key, NULL);
    }

    return result;
}

void free(void *ptr) {
    if (libc_free == NULL) jm_initialize();

    if ((pthread_getspecific(free_key)) == NULL) {
        pthread_setspecific(free_key, &free_key);

        if (ptr != NULL) {
            // TODO: ...
        }

        pthread_setspecific(free_key, NULL);
    }

    return libc_free(ptr);
}

/* ========================================================================> */

void calloc_init_once(void) {
    pthread_key_create(&calloc_key, NULL);

#ifdef __GLIBC__
    extern void *__libc_calloc(size_t num, size_t size);

    void *libc_calloc_ptr = __libc_calloc;
#else
    void *libc_calloc_ptr = dlsym(RTLD_NEXT, "calloc");
#endif

    libc_calloc = libc_calloc_ptr;

    assert(libc_calloc != NULL);
}

void malloc_init_once(void) {
    pthread_key_create(&malloc_key, NULL);

    void *libc_malloc_ptr = dlsym(RTLD_NEXT, "malloc");

    libc_malloc = libc_malloc_ptr;

    assert(libc_malloc != NULL);
}

void realloc_init_once(void) {
    pthread_key_create(&realloc_key, NULL);

    void *libc_realloc_ptr = dlsym(RTLD_NEXT, "realloc");

    libc_realloc = libc_realloc_ptr;

    assert(libc_realloc != NULL);
}

void free_init_once(void) {
    pthread_key_create(&free_key, NULL);

    void *libc_free_ptr = dlsym(RTLD_NEXT, "free");

    libc_free = libc_free_ptr;

    assert(libc_free != NULL);
}

/* ========================================================================> */

PRINTF_VISIBILITY void putchar_(char c) {
    write(1, &c, 1);
}
