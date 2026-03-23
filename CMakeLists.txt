cmake_minimum_required(VERSION 4.2.3)
project(packr)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

add_executable(packr src/main.c src/entry.c src/ops.c src/utils.c)
target_include_directories(packr PRIVATE include/)
target_compile_options(packr PRIVATE 
					$<$<CONFIG:Debug>: -g -O0>	
					$<$<CONFIG:Release>: -O2>	

)

include(FetchContent)
FetchContent_Declare(
	googletest
	  GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(tests ./tests/main.cpp ./src/utils.c ./src/entry.c)
include(GoogleTest)
target_link_libraries(tests PRIVATE GTest::gtest_main)
target_include_directories(tests PRIVATE include/)
gtest_discover_tests(tests)
