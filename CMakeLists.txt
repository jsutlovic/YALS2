cmake_minimum_required(VERSION 2.8)
project(YALS2)

set(CMAKE_VERBOSE_MAKEFILE off)

set(BIN_DIR ${YALS2_SOURCE_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O2")

add_subdirectory(src)

