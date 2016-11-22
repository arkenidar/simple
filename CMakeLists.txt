cmake_minimum_required(VERSION 3.5)
project(simple)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    simple.c
    )

add_executable(simple ${SOURCE_FILES})