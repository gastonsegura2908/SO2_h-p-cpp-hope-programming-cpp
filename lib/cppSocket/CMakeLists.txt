# Request the minimum version of CMake, in case of lower version throws an error.
cmake_minimum_required(VERSION 3.25 FATAL_ERROR)
# Define your project and specify the use of C++.
project(
  "SocketWrapper"
  VERSION 1.0.0
  DESCRIPTION "Socket Wrapper for c++"
  LANGUAGES CXX
)

# Define the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=maybe-uninitialized")

# Include the `include` directory, where the headers are located
include_directories(include)

# To automatically download external dependencies, use `FetchContent`
include(FetchContent)

# Enable debug
set(FETCHCONTENT_QUIET OFF)

# Add the `src` directory, where the C++ source files are located
file(GLOB_RECURSE SOURCES "src/*.cpp")  

# Create the executable
add_library(${PROJECT_NAME} ${SOURCES})

