cmake_minimum_required(VERSION 3.8)
project(Liso_Server)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES echo_server.c echo_client.c helper.h helper.c parse.c parse.h protocol.c protocol.h http_methods.h http_methods.c http_defs.h http_defs.c)
add_executable(Liso_Server ${SOURCE_FILES})