# Set minimum cmake version
cmake_minimum_required(VERSION 3.17)

# Set project name
project(UofC_CPSC_457_Spring_2021)

# Set C++ build flags
set(CMAKE_CXX_FLAGS "-O2 -Wall")

# Set verbose build
set(CMAKE_VERBOSE_MAKEFILE ON)

# Add C++ program executable tied to the C++ source file
add_executable(slow-pali Assignment1/slow-pali.cpp)
add_executable(fast-pali Assignment1/fast-pali.cpp)
