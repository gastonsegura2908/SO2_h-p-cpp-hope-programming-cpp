project(
  "rocksDBWrapper"
  VERSION 1.0.0
  DESCRIPTION "Header wrapper facilitating seamless integration of the RocksDB database into C++ projects."
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
include_directories(include)

file(GLOB_RECURSE SOURCES "src/rocksDbWrapper.cpp")

find_library(ROCKSDB_LIBRARY rocksdb)
find_path(ROCKSDB_INCLUDE_DIR rocksdb/db.h)

if (ROCKSDB_LIBRARY AND ROCKSDB_INCLUDE_DIR)
  message("RocksDB found")
else()
  if(NOT ROCKSDB_LIBRARY)
    message(FATAL_ERROR "RocksDB library not found")
  endif()
  if(NOT ROCKSDB_INCLUDE_DIR)
    message(FATAL_ERROR "RocksDB include directory not found")
  endif()
endif()

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC rocksdb) 


