cmake_minimum_required(VERSION 3.8)
project(Liso_Server)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES echo_server.c echo_client.c dbg_logger.h dbg_logger.c)
add_executable(Liso_Server ${SOURCE_FILES})