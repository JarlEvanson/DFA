cmake_minimum_required(VERSION 3.0.0)
project(Compiler VERSION 0.1.0)

#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pg")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_BUILD_PARALLEL_LEVEL 4)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_executable(
    DFA
    src/main.c 
    src/node.c 
    src/regex.c
    src/fileWriter.c
    src/scanner.c
    src/buffer.c
)

include_directories(DFA include)