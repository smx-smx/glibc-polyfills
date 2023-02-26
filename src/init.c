/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#include <stdlib.h>
#include "common.h"

bool _glibc_polyfills_debug = false;

static void
__attribute__((constructor(101)))
glibc_polyfills_init(){
	char *env = getenv("GLIBC_POLYFILLS_DEBUG");
	_glibc_polyfills_debug = (env && *env == '1');
}
