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

/* TODO: `aligned_alloc()`, `posix_memalign()`, `sbrk()`, `mmap()`, etc. */

typedef void *(jm_libc_dlopen)(const char *file, int mode);
typedef int (jm_libc_dlclose)(void *handle);

/* Private Variables ======================================================> */

static jm_libc_calloc_t *libc_calloc;
static jm_libc_malloc_t *libc_malloc;
static jm_libc_realloc_t *libc_realloc;

static jm_libc_free_t *libc_free;

/* ========================================================================> */

static jm_libc_dlopen *libc_dlopen;
static jm_libc_dlclose *libc_dlclose;

/* ========================================================================> */

static pthread_once_t calloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t malloc_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t realloc_key_once = PTHREAD_ONCE_INIT;

static pthread_once_t free_key_once = PTHREAD_ONCE_INIT;

/* ========================================================================> */

static pthread_once_t dlopen_key_once = PTHREAD_ONCE_INIT;
static pthread_once_t dlclose_key_once = PTHREAD_ONCE_INIT;

/* ========================================================================> */

static pthread_key_t calloc_key;
static pthread_key_t malloc_key;
static pthread_key_t realloc_key;

static pthread_key_t free_key;

/* Private Function Prototypes ============================================> */

static void jm_preload_calloc_init(void);
static void jm_preload_calloc_deinit(void);

static void jm_preload_malloc_init(void);
static void jm_preload_malloc_deinit(void);

static void jm_preload_realloc_init(void);
static void jm_preload_realloc_deinit(void);

/* ========================================================================> */

static void jm_preload_free_init(void);
static void jm_preload_free_deinit(void);

/* ========================================================================> */

static void jm_preload_dlopen_init(void);
static void jm_preload_dlopen_deinit(void);

static void jm_preload_dlclose_init(void);
static void jm_preload_dlclose_deinit(void);

/* Public Functions =======================================================> */

void jm_preload_init(void) {
    (void) pthread_once(&calloc_key_once, jm_preload_calloc_init);
    (void) pthread_once(&malloc_key_once, jm_preload_malloc_init);
    (void) pthread_once(&realloc_key_once, jm_preload_realloc_init);
    
    (void) pthread_once(&free_key_once, jm_preload_free_init);

    (void) pthread_once(&dlopen_key_once, jm_preload_dlopen_init);
    (void) pthread_once(&dlclose_key_once, jm_preload_dlclose_init);
}

void jm_preload_deinit(void) {
    (void) pthread_once(&calloc_key_once, jm_preload_calloc_deinit);
    (void) pthread_once(&malloc_key_once, jm_preload_malloc_deinit);
    (void) pthread_once(&realloc_key_once, jm_preload_realloc_deinit);
    
    (void) pthread_once(&free_key_once, jm_preload_free_deinit);

    (void) pthread_once(&dlopen_key_once, jm_preload_dlopen_deinit);
    (void) pthread_once(&dlclose_key_once, jm_preload_dlclose_deinit);
}

/* ========================================================================> */

void *calloc(size_t num, size_t size) {
    if (libc_calloc == NULL) jm_tracker_init();

    void *result = libc_calloc(num, size);

    if ((pthread_getspecific(calloc_key)) == NULL) {
        pthread_setspecific(calloc_key, &calloc_key);

        // TODO: ...

        pthread_setspecific(calloc_key, NULL);
    }

    return result;
}

void *malloc(size_t size) {
    if (libc_malloc == NULL) jm_tracker_init();

    void *result = libc_malloc(size);

    if ((pthread_getspecific(malloc_key)) == NULL) {
        pthread_setspecific(malloc_key, &malloc_key);

        // TODO: ...

        pthread_setspecific(malloc_key, NULL);
    }

    return result;
}

void *realloc(void *ptr, size_t new_size) {
    if (libc_realloc == NULL) jm_tracker_init();

    void *result = libc_realloc(ptr, new_size);

    if ((pthread_getspecific(realloc_key)) == NULL) {
        pthread_setspecific(realloc_key, &realloc_key);

        // TODO: ...

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
    if (libc_free == NULL) jm_tracker_init();

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

void *dlopen(const char *file, int mode) {
    if (libc_dlopen == NULL) jm_tracker_init();

    /* TODO: ... */

    return libc_dlopen(file, mode);
}

int dlclose(void *handle) {
    if (libc_dlclose == NULL) jm_tracker_init();

    /* TODO: ... */

    return libc_dlclose(handle);
}

/* ========================================================================> */

PRINTF_VISIBILITY void putchar_(char c) {
    write(1, &c, 1);
}

/* Private Functions =====================================================> */

static void jm_preload_calloc_init(void) {
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

static void jm_preload_calloc_deinit(void) {
    pthread_key_delete(calloc_key);
}

static void jm_preload_malloc_init(void) {
    pthread_key_create(&malloc_key, NULL);

    void *libc_malloc_ptr = dlsym(RTLD_NEXT, "malloc");

    libc_malloc = libc_malloc_ptr;

    assert(libc_malloc != NULL);
}

static void jm_preload_malloc_deinit(void) {
    pthread_key_delete(malloc_key);
}

static void jm_preload_realloc_init(void) {
    pthread_key_create(&realloc_key, NULL);

    void *libc_realloc_ptr = dlsym(RTLD_NEXT, "realloc");

    libc_realloc = libc_realloc_ptr;

    assert(libc_realloc != NULL);
}

static void jm_preload_realloc_deinit(void) {
    pthread_key_delete(realloc_key);
}

static void jm_preload_free_init(void) {
    pthread_key_create(&free_key, NULL);

    void *libc_free_ptr = dlsym(RTLD_NEXT, "free");

    libc_free = libc_free_ptr;

    assert(libc_free != NULL);
}

static void jm_preload_free_deinit(void) {
    pthread_key_delete(free_key);
}

/* ========================================================================> */

static void jm_preload_dlopen_init(void) {
    void *libc_dlopen_ptr = dlsym(RTLD_NEXT, "dlopen");

    libc_dlopen = libc_dlopen_ptr;

    assert(libc_dlopen != NULL);
}

static void jm_preload_dlopen_deinit(void) {
    // TODO: ...
}

static void jm_preload_dlclose_init(void) {
    void *libc_dlclose_ptr = dlsym(RTLD_NEXT, "dlclose");

    libc_dlclose = libc_dlclose_ptr;

    assert(libc_dlclose != NULL);
}

static void jm_preload_dlclose_deinit(void) {
    // TODO: ...
}
