cmake_minimum_required(VERSION 3.10)

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if(POLICY CMP0025)
	cmake_policy(SET CMP0025 NEW)
endif()

project(comp3931test CXX)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable all compiler warnings
if(MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
endif()

# Build the components
include_directories(.)
add_executable(COMP3911Test TestApplication.cpp JACKCompiler.cpp)
