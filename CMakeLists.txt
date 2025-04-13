cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

set(CMAKE_POLICY_DEFAULT_CMO0077 NEW)

project(
  "HPCPP"
  VERSION 1.0.0
  DESCRIPTION "High Performance C++"
  LANGUAGES CXX 
)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(WITH_BENCHMARK_TOOLS OFF)
set(WITH_TOOLS OFF)
set(WITH_TESTS OFF)

include_directories(include)

include(FetchContent)

set(GTEST_GIT_URL "https://github.com/google/googletest.git")
set(ROCKSDB_GIT_URL "https://github.com/facebook/rocksdb.git")
set(NLOHMANN_JSON_GIT_URL "https://github.com/nlohmann/json.git")
set(CPP_HTTPLIB_GIT_URL "https://github.com/yhirose/cpp-httplib.git")
set(JWT_CPP_GIT_URL "https://github.com/Thalhammer/jwt-cpp.git")


FetchContent_Declare(
  googletest
  GIT_REPOSITORY ${GTEST_GIT_URL}
  GIT_TAG release-1.12.0  # Optionally pin to a stable release
)

set(WITH_BENCHMARK_TOOLS OFF)
set(WITH_TOOLS OFF)
set(WITH_TESTS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(cpp_httplib)

#RocksDB
find_package(RocksDB)
if(ROCKSDB_FOUND)
    set(ROCKSDB_LIB RocksDB::rocksdb)
else()
    message(STATUS "RocksDB not found in the system, fetching from GitHub")
    FetchContent_Declare(
    rocksdb
    GIT_REPOSITORY ${ROCKSDB_GIT_URL}
    GIT_TAG v6.22.1
    CMAKE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPORTABLE=OFF -DROCKSDB_CMAKE_CXX_FLAGS="-O2"
    )

    FetchContent_MakeAvailable(rocksdb)
    set(ROCKSDB_LIB Rocksdb::Rocksdb)
endif()

set(SERVER_SOURCE "src/server.cpp")
set(CLIENT_SOURCE "src/client.cpp") 
set(TIMER_SOURCE "src/timer.cpp") 

set(SERVER_NAME "server")
set(CLIENT_NAME "client")

add_executable(${SERVER_NAME} ${SERVER_SOURCE})
add_executable(${CLIENT_NAME}  ${CLIENT_SOURCE})

#OpenMP
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${SERVER_NAME} INTERFACE OpenMP::OpenMP_CXX)
else()
    message(FATAL_ERROR "OpenMP not found")
endif()

target_include_directories(${SERVER_NAME} PUBLIC ${httplib_SOURCE_DIR}/include)
target_include_directories(${CLIENT_NAME} PUBLIC ${httplib_SOURCE_DIR}/include)
target_include_directories(${SERVER_NAME} PUBLIC lib/libmodules/include)
target_include_directories(${SERVER_NAME} PUBLIC lib/rocksDbWrapper/include)
target_include_directories(${SERVER_NAME} PUBLIC lib/cppSocket/include)
target_include_directories(${CLIENT_NAME} PUBLIC lib/cppSocket/include)
target_include_directories(${SERVER_NAME} PUBLIC lib/cannyEdgeFilter/include)

add_subdirectory(lib/libmodules) 
add_subdirectory(lib/rocksDbWrapper)
add_subdirectory(lib/cppSocket)
add_subdirectory(lib/cannyEdgeFilter)


target_link_libraries(${SERVER_NAME} PUBLIC httplib nlohmann_json LibModules SocketWrapper cannyEdge rocksDBWrapper)
target_link_libraries(${CLIENT_NAME} PUBLIC httplib nlohmann_json SocketWrapper )

if (RUN_TESTS EQUAL 1 OR RUN_COVERAGE EQUAL 1)
    add_subdirectory(tests)
endif()

