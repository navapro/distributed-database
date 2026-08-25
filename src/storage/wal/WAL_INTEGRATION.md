# WAL — integration instructions (for Yashila)

The WAL is built, tested, and passing against Naveen's real `Record` with real
GoogleTest. This file tells you exactly what to add so it builds inside the repo.

## New files (drop-in, already in correct paths)

    src/storage/wal/wal.h
    src/storage/wal/crc32.h
    src/storage/wal/wal.cpp
    tests/unit/wal_test.cpp

Nothing else changes. I did NOT touch `src/common/record.{h,cpp}` — those stay
Naveen's.

## CMake edits

Your `tests/CMakeLists.txt` already links a target called `quorumdb_common` and
builds one test exe called `quorumdb_tests`. That means the ROOT
`CMakeLists.txt` (not shared with me) defines `quorumdb_common` and adds the
`tests/` subdirectory. So the WAL needs two small edits.

### 1. Root `CMakeLists.txt`  — add the WAL to the common library

Find where `quorumdb_common` is defined. It will look roughly like:

    add_library(quorumdb_common
        src/common/record.cpp
    )
    target_include_directories(quorumdb_common PUBLIC src)

Add the WAL source to that same library. Change it to:

    add_library(quorumdb_common
        src/common/record.cpp
        src/storage/wal/wal.cpp        # <-- ADD THIS LINE
    )
    target_include_directories(quorumdb_common PUBLIC src)

That's the only root edit. (`crc32.h` and `wal.h` are headers — they don't get
listed as sources; they're found via the `src` include dir.)

If your root file names the library something other than `quorumdb_common`, add
the line to whatever library `tests/CMakeLists.txt` links against instead.

### 2. `tests/CMakeLists.txt`  — add the WAL test file

You currently have:

    add_executable(quorumdb_tests
        unit/record_test.cpp
    )

Change it to:

    add_executable(quorumdb_tests
        unit/record_test.cpp
        unit/wal_test.cpp              # <-- ADD THIS LINE
    )

Nothing else in that file changes — it already links `GTest::gtest_main` and
calls `gtest_discover_tests`, which will pick up the 5 new `WalTest.*` cases
automatically.

## Build & run

    cmake -S . -B build
    cmake --build build
    cd build && ctest --output-on-failure

You should see the 3 existing `RecordTest.*` cases plus 5 new `WalTest.*` cases,
all passing.

## One thing to confirm with Naveen at the checkpoint

The WAL payload byte order is:

    timestamp(8) | tombstone(1) | key_len(4) | key | value_len(4) | value

docs/storage-record-format.md lists these fields as required but doesn't pin an
order. Make sure the SSTable encoder uses the SAME order (or that you both
agree they can differ, since WAL and SSTable are separate files). This is the
kind of mismatch that only shows up when his SSTable reader tries to read data
your WAL wrote — so lock it down now.
