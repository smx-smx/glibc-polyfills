cmake_minimum_required(VERSION 3.0)

include_directories(src)
add_subdirectory(src)

add_library(glibc_polyfills
	$<TARGET_OBJECTS:init>
	$<TARGET_OBJECTS:glibc_2_16>
)