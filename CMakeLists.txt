cmake_minimum_required(VERSION 3.15)
project(Nautyyy)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(Nautyyy
        adjacencymatrix.cpp
        adjacencymatrix.h
        main.cpp
        nautyyy.cpp
        nautyyy.h
        "partition and refinement.cpp"
        "partition and refinement.h"
        "permutation group.cpp"
        "permutation group.h")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")