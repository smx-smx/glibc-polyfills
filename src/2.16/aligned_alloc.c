/**
 * Copyright 2023 throwaway96
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <dlfcn.h>
#include <malloc.h>

#include "common.h"

/** function prototypes **/
static void *_aligned_alloc_init_wrapper(size_t alignment, size_t size);

/** types **/
typedef void *(*pfn_aligned_alloc)(size_t alignment, size_t size);

/** globals **/
static pfn_aligned_alloc aligned_alloc_fn = _aligned_alloc_init_wrapper;

#ifdef ALIGNED_ALLOC_C17_SEMANTICS
# define DEFAULT_ALIGNED_ALLOC_C17_SEMANTICS true
#else
# define DEFAULT_ALIGNED_ALLOC_C17_SEMANTICS false
#endif
static const bool aligned_alloc_default_c17_semantics = DEFAULT_ALIGNED_ALLOC_C17_SEMANTICS;

/* borrowed from glibc */
#define powerof2(x)     ((((x) - 1) & (x)) == 0)

static void *_aligned_alloc_polyfill_c17(size_t alignment, size_t size) {
    if (!powerof2(alignment) || (alignment == 0)) {
        errno = EINVAL;
        return NULL;
    }

    return memalign(alignment, size);
}

static void *_aligned_alloc_init_wrapper(size_t alignment, size_t size) {
    DBG("aligned_alloc", "init\n");

    bool c17_semantics = aligned_alloc_default_c17_semantics;
    const char *env;

    if ((env = getenv("GLIBC_POLYFILLS_ALIGNED_ALLOC_C17_SEMANTICS")) != NULL) {
        c17_semantics = (env[0] == '1');

        DBG("aligned_alloc", "c17_semantics: default=%d, env=%d\n",
            aligned_alloc_default_c17_semantics, c17_semantics);
    }

    pfn_aligned_alloc fptr = dlsym(RTLD_NEXT, "aligned_alloc");

    if (fptr == NULL) {
            /* aligned_alloc wasn't found */

        if (c17_semantics) {
            fptr = &_aligned_alloc_polyfill_c17;
        } else {
            fptr = &memalign;
        }
    }

    aligned_alloc_fn = fptr;

    return aligned_alloc_fn(alignment, size);
}

void *aligned_alloc(size_t alignment, size_t size) {
    return aligned_alloc_fn(alignment, size);
}
