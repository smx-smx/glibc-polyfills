cmake_minimum_required(VERSION 3.0)
project(glibc-polyfills LANGUAGES C)

include_directories(src)
add_subdirectory(src)

add_library(glibc_polyfills
	$<TARGET_OBJECTS:init>
	$<TARGET_OBJECTS:glibc_2_16>
)

install(TARGETS glibc_polyfills)
