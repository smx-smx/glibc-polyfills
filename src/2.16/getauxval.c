/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

/** types **/
struct auxv_entry {
	unsigned long type;
	unsigned long value;
};
typedef unsigned long (*pfn_getauxval)(unsigned long type);

/** globals **/
static struct auxv_entry *g_auxv = NULL;;
static pfn_getauxval getauxval_fn = NULL;

static unsigned long _getauxval(unsigned long type){
	struct auxv_entry *a;
	for(a = g_auxv; a->type != 0 && a->type != type; ++a);
	return (a->type == 0) ? 0 : a->value;
}


static int _getauxval_init(){
	int fd = open("/proc/self/auxv", O_RDONLY);
	if(fd < 0) return -1;

	struct auxv_entry e = {0,0};

	int num_auxv;
	// look for the end (always include AT_NULL)
	for(num_auxv = 1;
		read(fd, &e, sizeof(e)) == sizeof(e)
		&& e.type != 0
		&& e.value != 0;
		++num_auxv
	);

	if(num_auxv < 1) return -1;

	lseek(fd, 0, SEEK_SET);

	g_auxv = calloc(num_auxv, sizeof(e));
	for(int i=0; i<num_auxv; i++){
		if(read(fd, &g_auxv[i], sizeof(e)) != sizeof(e)) break;
	}

	close(fd);
	return 0;
}

static void __attribute__((constructor))
getauxval_init(){
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