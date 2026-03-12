cmake_minimum_required(VERSION 3.31.7)

project(packr)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(packr src/main.c)

target_include_directories(packr PRIVATE include/)
