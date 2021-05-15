# Set minimum cmake version
cmake_minimum_required(VERSION 3.17)

# Force specific compiler for C files
#set(CMAKE_C_COMPILER "gcc")

# Force specific compiler for C++ files
#set(CMAKE_CXX_COMPILER "g++")

# Set project name
project(UofC_CPSC_457_Spring_2021)

# Set C++ build standard
#set(CMAKE_CXX_STANDARD 14)

# Set C++ build flags
set(CMAKE_CXX_FLAGS "-O2 -Wall")

# Set verbose build
set(CMAKE_VERBOSE_MAKEFILE ON)

# Set the runtime directory prior to executing a program
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Add C++ program executable tied to the C++ source file
add_executable(slow-pali Assignment1/slow-pali.cpp)
add_executable(fast-pali Assignment1/fast-pali.cpp)
