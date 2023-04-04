/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <link.h>

#include "common.h"

/** function prototypes **/
static unsigned long _getauxval_init_wrapper(unsigned long type);

/** types **/
typedef unsigned long (*pfn_getauxval)(unsigned long type);

/** globals **/
static ElfW(auxv_t) *g_auxv = NULL;
static pfn_getauxval getauxval_fn = _getauxval_init_wrapper;

#ifdef SKIP_GLIBC
# define DEFAULT_SKIP_GLIBC true
#else
# define DEFAULT_SKIP_GLIBC false
#endif
static const bool getauxval_default_skip_glibc = DEFAULT_SKIP_GLIBC;

static unsigned long _getauxval_polyfill(unsigned long type) {
	ElfW(auxv_t) *p;

	for (p = g_auxv; p->a_type != AT_NULL; p++) {
		if (p->a_type == type) {
			return p->a_un.a_val;
		}
	}

	errno = ENOENT;
	return 0;
}

static bool _getauxval_read(int *errsv) {
	int fd;

	if ((fd = open("/proc/self/auxv", O_RDONLY)) == -1) {
		if (errsv != NULL) {
			*errsv = errno;
		}

		return false;
	}

	ElfW(auxv_t) pairs[256];
	ssize_t rlen;

	if ((rlen = read(fd, pairs, sizeof(pairs))) == -1) {
		if (errsv != NULL) {
			*errsv = errno;
		}

		close(fd);
		return false;
	}

	close(fd);

	DBG_EXPR({
		size_t n = (size_t)rlen / sizeof(pairs[0]);

		for (unsigned int i = 0; i < n; i++) {
			DBG("getauxval", "0x%x -> 0x%x\n", pairs[i].a_type, pairs[i].a_un.a_val);
		}
	});

	if ((g_auxv = malloc(rlen)) == NULL) {
		if (errsv != NULL) {
			*errsv = errno;
		}

		return false;
	}

	memcpy(g_auxv, pairs, rlen);

	return true;
}


static unsigned long _getauxval_init_wrapper(unsigned long type) {
	DBG("getauxval", "init\n");

	bool skip_glibc = getauxval_default_skip_glibc;
	const char *env;

	if ((env = getenv("GLIBC_POLYFILLS_GETAUXVAL_SKIP_GLIBC")) != NULL) {
		skip_glibc = (env[0] == '1');

		DBG("getauxval", "skip_glibc: default=%d, env=%d\n", getauxval_default_skip_glibc, skip_glibc);
	}

	pfn_getauxval fptr = NULL;

	if (!skip_glibc) {
		fptr = dlsym(RTLD_NEXT, "getauxval");
	}

	if (fptr == NULL) {
		/* either glibc wasn't searched or getauxval wasn't found */
		int errsv = 0;

		if (!_getauxval_read(&errsv)) {
			fprintf(stderr, "_getauxval_read() failed: %s\n", strerror(errsv));
			abort();
		}

		fptr = &_getauxval_polyfill;
	}

	getauxval_fn = fptr;

	return getauxval_fn(type);
}

static __attribute__((destructor)) void _getauxval_dtor(void) {
	if (g_auxv != NULL) {
		free(g_auxv);
		g_auxv = NULL;
	}
}

unsigned long getauxval(unsigned long type) {
	return getauxval_fn(type);
}
