cmake_minimum_required(VERSION 3.25)

project(local_server C)

include_directories(lib)

add_executable(local_server src/server.c lib/common.c)