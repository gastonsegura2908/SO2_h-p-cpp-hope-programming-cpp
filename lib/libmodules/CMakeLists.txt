cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(
    "LibModules"
    VERSION 1.0.0
    DESCRIPTION "Alert and emergency Modules"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_library(${PROJECT_NAME} STATIC ${SOURCES})


