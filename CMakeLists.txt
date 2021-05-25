# Set minimum cmake version
cmake_minimum_required(VERSION 3.17)

# Set project name
project(UofC_CPSC_457_Spring_2021)

# Set C++ build flags
set(CMAKE_CXX_FLAGS "-O2 -Wall")

# Set verbose build
set(CMAKE_VERBOSE_MAKEFILE ON)

# Links the library used for Assignment 2
link_libraries(-lcrypto)

# Add C++ program executable tied to the C++ source file
add_executable(A1_slow-pali Assignment1/slow-pali.cpp)
add_executable(A1_fast-pali Assignment1/fast-pali.cpp)
add_executable(A2_main Assignment2/main.cpp Assignment2/digester.cpp Assignment2/getDirStats.cpp)
