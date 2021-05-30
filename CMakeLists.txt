# Set minimum cmake version
cmake_minimum_required(VERSION 3.17)

# Set project name
project(UofC_CPSC_457_Spring_2021)

# Set C++ build flags
set(CMAKE_CXX_FLAGS "-O2 -Wall")

# Set verbose build
set(CMAKE_VERBOSE_MAKEFILE ON)

# Links the library that will be used
link_libraries(-lcrypto) # Assignment 2
link_libraries(-pthread) # Assignment 3 (calcpi & detectPrimes)
link_libraries(-lm) # Assignment 3 (detectPrimes)

# Add C++ program executable tied to the C++ source file
add_executable(A1_slow-pali Assignment1/slow-pali.cpp)
add_executable(A1_fast-pali Assignment1/fast-pali.cpp)
add_executable(A2_main Assignment2/main.cpp Assignment2/digester.cpp Assignment2/getDirStats.cpp)
add_executable(A3_calcpi Assignment3/pi-calc/main.cpp Assignment3/pi-calc/calcpi.cpp)
add_executable(A3_detectPrimes Assignment3/detectPrimes/main.cpp Assignment3/detectPrimes/detectPrimes.cpp)
