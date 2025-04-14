cmake_minimum_required(VERSION 3.25 FATAL_ERROR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

file(GLOB TESTS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp)


file(GLOB SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libmodules/src/AlertInvasion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libmodules/src/EmergencyNotification.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libmodules/src/SuppliesData.cpp
)

add_compile_definitions(TEST)

if(RUN_COVERAGE)
    message("Coverage enabled!")
    find_program(LCOV_PATH lcov)
        if(NOT LCOV_PATH)
        message(FATAL_ERROR "lcov not fund! aborting...")
    endif()

    find_program(GCOV_PATH gcov)
        if(NOT GCOV_PATH)
        message(FATAL_ERROR "gcov not fund! aborting...")
    endif()

    find_program(GENHTML_PATH genhtml)
    if(NOT GENHTML_PATH)
        message(FATAL_ERROR "gcov not fund! aborting...")
    endif()

endif ()

add_compile_options(-g -fprofile-arcs -ftest-coverage --coverage)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov --coverage")

add_executable(test_${PROJECT_NAME} ${TESTS_FILES} ${SRC_FILES})

target_link_libraries(test_${PROJECT_NAME} 
    gtest 
    gtest_main
    httplib 
    LibModules
    nlohmann_json 
    rocksDBWrapper 
    SocketWrapper 
    cannyEdge
    OpenMP::OpenMP_CXX
)
add_test(NAME test_${PROJECT_NAME} COMMAND test_${PROJECT_NAME})

target_include_directories(test_${PROJECT_NAME} PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR}/../lib/libmodules/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/cannyEdgeFilter/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/rocksDbWrapper/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/cppSocket/include

)
include(GoogleTest)
gtest_discover_tests(test_${PROJECT_NAME})


