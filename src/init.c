/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#include <stdlib.h>
#include "common.h"

bool _glibc_polyfill_debug = false;

static void
__attribute__((constructor(0)))
glibc_polyfill_init(){
	char *env = getenv("GLIBC_POLYFILL_DEBUG");
	_glibc_polyfill_debug = (env && *env == '1');
}
