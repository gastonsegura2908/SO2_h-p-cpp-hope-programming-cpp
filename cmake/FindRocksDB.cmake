# Locate RocksDB library
# This module searches for RocksDB library on the system

# Try to find RocksDB in standard installation paths
find_library(ROCKSDB_LIBRARY
    NAMES rocksdb
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
)

# Try to find RocksDB include directory
find_path(ROCKSDB_INCLUDE_DIR
    NAMES rocksdb/db.h
    PATH_SUFFIXES rocksdb
    PATHS /usr/include /usr/local/include 
)

# Set variables indicating RocksDB status and location
if (ROCKSDB_LIBRARY AND ROCKSDB_INCLUDE_DIR)
    set(ROCKSDB_FOUND TRUE)
    message(STATUS "RocksDB found: ${ROCKSDB_LIBRARY}, inc: ${ROCKSDB_INCLUDE_DIR}")
else()
    set(ROCKSDB_FOUND FALSE)
    message(STATUS "RocksDB not found")
endif()

# Provide RocksDB library and include directory to the caller
if (ROCKSDB_FOUND)
    if (NOT TARGET RocksDB::RocksDB)
        add_library(RocksDB::RocksDB UNKNOWN IMPORTED)
        set_target_properties(RocksDB::RocksDB PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${ROCKSDB_INCLUDE_DIR}"
            IMPORTED_LOCATION "${ROCKSDB_LIBRARY}"
        )
    endif()
endif()

