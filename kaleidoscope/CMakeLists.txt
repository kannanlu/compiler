cmake_minimum_required(VERSION 3.10)

# Project name and language
project(Kaleidoscope VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
set(SOURCES
    kaleidoscope/utils/frontend.cpp
)

# Create executable
add_executable(kaleidoscope ${SOURCES})

# Add include directories if needed
target_include_directories(kaleidoscope
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set compiler warnings
if(MSVC)
    target_compile_options(kaleidoscope PRIVATE /W4)
else()
    target_compile_options(kaleidoscope PRIVATE -Wall -Wextra -Wpedantic)
endif()