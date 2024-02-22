cmake_minimum_required(VERSION 3.20.0)

PROJECT(dir_diff)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_compile_options(-std=c++17)
add_compile_options(-O3)
add_compile_options(-Wall)

add_executable(dir_diff dir_diff.cpp)
