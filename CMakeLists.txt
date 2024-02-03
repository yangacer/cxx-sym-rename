cmake_minimum_required(VERSION 3.4)

project(CXXSymRename CXX)

set(CMAKE_CXX_STANDARD 11)
add_executable(cxx-sym-rename demangle.cpp main.cpp)
