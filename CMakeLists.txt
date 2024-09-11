# CMakeList.txt: SPP 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)
project(SPP)

include_directories("/usr/include/eigen3")
file(GLOB_RECURSE SRC_LIST "src/*.cpp")
set(CMAKE_CXX_STANDARD 11)
add_executable (SPP "main.cpp" ${SRC_LIST})

