/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <link.h>

#include "common.h"

/** types **/

typedef unsigned long (*pfn_getauxval)(unsigned long type);

/** globals **/
static ElfW(auxv_t) *g_auxv = NULL;;
static pfn_getauxval getauxval_fn = NULL;

static unsigned long _getauxval(unsigned long type){
	ElfW(auxv_t) *a;
	for(a = g_auxv; a->a_type != 0 && a->a_type != type; ++a);
	return (a->a_type == 0) ? 0 : a->a_un.a_val;
}

static int _getauxval_init(){
	ElfW(auxv_t) pairs[256];
	
	int fd = open ("/proc/self/auxv", O_RDONLY);
	if(fd < 0) return -1;

	ssize_t rlen = read (fd, pairs, sizeof(pairs));
	close (fd);
  	if (rlen < 0) return -1;

	size_t n = (size_t)rlen / sizeof(pairs[0]);

	DBG_EXPR({
		for(int i=0; i<n; i++){
			DBG("getauxval", "0x%x -> 0x%x\n", pairs[i].a_type, pairs[i].a_un.a_val);
		}
	});

	g_auxv = malloc(rlen);
	memcpy(g_auxv, pairs, rlen);
	return 0;
}


static void
__attribute__((constructor))
getauxval_init(){
	DBG("getauxval", "init\n");

#ifndef SKIP_GLIBC
	void *glibc_getauxval = dlsym(RTLD_NEXT, "getauxval");
	if(glibc_getauxval != NULL){
		getauxval_fn = glibc_getauxval;
	} else
#endif
	{
		if(_getauxval_init() < 0){
			fputs("getauxval_init() failed\n", stderr);
			abort();
		}
		getauxval_fn = &_getauxval;
	}
}

static void __attribute__((destructor))
getauxval_deinit(){
	if(g_auxv != NULL){
		free(g_auxv);
		g_auxv = NULL;
	}
}


unsigned long getauxval(unsigned long type){
	return getauxval_fn(type);
}
