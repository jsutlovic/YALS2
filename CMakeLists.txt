cmake_minimum_required(VERSION 2.8)
project(YALS2)

set(CMAKE_VERBOSE_MAKEFILE off)

set(BIN_DIR ${YALS2_SOURCE_DIR}/bin)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -std=c11 -funroll-loops")
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-no_pie -msse4.2")
endif()
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-g -O3")

add_subdirectory(src)

