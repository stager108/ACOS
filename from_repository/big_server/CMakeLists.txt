cmake_minimum_required(VERSION 3.4)
project(big_server)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(SOURCE_FILES
    big_server.c)

add_executable(big_server ${SOURCE_FILES})