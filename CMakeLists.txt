cmake_minimum_required(VERSION 3.20)

project(QuorumDB VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(QUORUMDB_BUILD_TESTS "Build Quorum DB tests" ON)

add_library(quorumdb_common
    src/common/record.cpp
)

target_include_directories(quorumdb_common PUBLIC src)

add_library(quorumdb_storage
    src/storage/memtable.cpp
)

target_include_directories(quorumdb_storage PUBLIC src)
target_link_libraries(quorumdb_storage PUBLIC quorumdb_common)

if(QUORUMDB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
