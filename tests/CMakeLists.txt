include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(quorumdb_tests
    unit/memtable_test.cpp
    unit/record_test.cpp
)

target_link_libraries(
    quorumdb_tests
    PRIVATE
        quorumdb_storage
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(quorumdb_tests)
