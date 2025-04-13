cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

project(
  "HPCPP"
  VERSION 1.0.0
  DESCRIPTION "High Performance C++"
  LANGUAGES CXX 
)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

include(FetchContent)

set(GTEST_GIT_URL "https://github.com/google/googletest.git")
set(ROCKSDB_GIT_URL "https://github.com/facebook/rocksdb.git")
set(NLOHMANN_JSON_GIT_URL "https://github.com/nlohmann/json.git")
set(CPP_HTTPLIB_GIT_URL "https://github.com/yhirose/cpp-httplib.git")

FetchContent_Declare(
  googletest
  GIT_REPOSITORY ${GTEST_GIT_URL}
  GIT_TAG release-1.12.0  # Optionally pin to a stable release
)

FetchContent_Declare(
  rocksdb
  GIT_REPOSITORY ${ROCKSDB_GIT_URL}
  GIT_TAG v9.0.0
)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY ${NLOHMANN_JSON_GIT_URL}
  GIT_TAG v3.11.2
)

FetchContent_Declare(
  cpp_httplib
  GIT_REPOSITORY ${CPP_HTTPLIB_GIT_URL}
  GIT_TAG v0.15.3
)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(rocksdb)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(cpp_httplib)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

#OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} INTERFACE OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()


if (RUN_TESTS EQUAL 1 OR RUN_COVERAGE EQUAL 1)
    add_subdirectory(tests)

    # Link Google Test to the test executable
    add_executable(unit_tests ${TEST_SOURCES})
    target_link_libraries(unit_tests gtest_main gmock_main)
    target_include_directories(unit_tests PUBLIC ${gtest_SOURCE_DIR}/include ${gmock_SOURCE_DIR}/include)

    # Add the tests to be run by CTest
    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()
