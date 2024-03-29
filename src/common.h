/**
 * Copyright(C) 2023 Stefano Moioli <smxdev4@gmail.com>
 */
#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <stdbool.h>

extern bool _glibc_polyfills_debug;

#define LOG(fmt, ...) ({ \
    if (_glibc_polyfills_debug) fprintf(stderr, fmt, ##__VA_ARGS__); \
})

#define DBG(module, fmt, ...) LOG("[" module "]: " fmt, ##__VA_ARGS__)

#define DBG_EXPR(v) if(_glibc_polyfills_debug) v

#endif
