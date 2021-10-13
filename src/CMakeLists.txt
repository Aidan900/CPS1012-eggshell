cmake_minimum_required(VERSION 3.9)
project(eggshell C)

set(CMAKE_C_STANDARD 99)

add_library(dynamicArrayLib dynamicArray.h dynamicArray.c)

add_library(stackLib stackLib.h stackLib.c)

add_executable(eggshell main.c linenoise.c redirectionHandler.c variables.c piping.c externalCmds.c internalCmds.c)
target_link_libraries(eggshell dynamicArrayLib stackLib)