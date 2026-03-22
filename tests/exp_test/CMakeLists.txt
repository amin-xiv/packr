cmake_minimum_required(VERSION 4.2.3)
project(packr)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

add_executable(test1 test1.c ./../../src/entry.c ./../../src/ops.c ./../../src/utils.c)
target_include_directories(test1 PRIVATE ../../include/)
target_compile_options(test1 PRIVATE 
					$<$<CONFIG:Debug>:-g -O0>	
					$<$<CONFIG:Release>:-O2>	

)
