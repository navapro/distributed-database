# Development Guide

This document defines the shared development contract for Quorum DB. It keeps builds, code reviews, and module interfaces consistent while Naveen and Yashila work concurrently.

## Toolchain

- Language: C++20
- Build system: CMake 3.20 or newer
- Test runner: CTest
- Test framework: GoogleTest, pinned to an exact release
- Supported compilers:
  - GCC 11 or newer
  - Clang 14 or newer
  - Apple Clang with C++20 support
- Primary deployment target: Linux containers
- Local development targets: Linux and macOS

Code must compile without compiler-specific language extensions. Platform-specific networking or filesystem code must stay behind a small interface and have a portable fallback or test double.

## Build Approach

The root `CMakeLists.txt` will define the project and add each module as a separate target. Production logic should live in libraries; executables should contain only startup, configuration, and command dispatch code.

Planned target categories:

- Storage library
- Cluster and replication libraries
- Networking library
- Quorum DB node executable
- Client and control CLI executables
- Unit and integration test executables
- Benchmark executable

Once the build files exist, the standard local workflow will be:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DQUORUMDB_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Release and sanitizer builds must use separate build directories such as `build-release` and `build-asan`. Generated build files must not be committed.

## Compiler Settings

All Quorum DB targets should enable a common warning set:

- GCC and Clang: `-Wall -Wextra -Wpedantic`
- Warnings are treated as errors in continuous integration once the initial build is stable.
- Debug builds include assertions and debug symbols.
- Sanitizer builds are optional debugging tools.

Compiler flags must be attached to Quorum DB targets rather than set globally so external dependencies do not inherit project warnings.

## Coding Conventions

- Files and directories use `snake_case`.
- Namespaces use lowercase `snake_case` under the top-level `quorumdb` namespace.
- Types and enums use `PascalCase`.
- Functions and local variables use `snake_case`.
- Private data members use `snake_case_` with a trailing underscore.
- Compile-time constants use `kPascalCase`.
- Header files use `#pragma once`.
- Public APIs use explicit types; `auto` is appropriate when the type is obvious or avoids repetition.
- Prefer `std::string_view` and spans for non-owning inputs, but never store them beyond the lifetime of their source.
- Use fixed-width integer types for persisted and networked data.

Formatting will be enforced by a checked-in `.clang-format` file. Until it is added, use four-space indentation, no tabs, braces on the same line, and a 100-column target.

## C++ Safety Rules

- Use RAII for files, sockets, locks, threads, and other resources.
- Prefer values and smart pointers; owning raw pointers are not allowed.
- Use `std::unique_ptr` for single ownership and `std::shared_ptr` only when ownership is genuinely shared.
- Use `std::chrono` types for durations and timestamps in runtime code.
- Use `std::filesystem::path` for filesystem paths.
- Do not use exceptions for expected storage, network, or validation failures. Return a typed status or result instead.
- Exceptions may be used for unrecoverable startup failures at executable boundaries.
- Document thread-safety expectations on every shared class.
- Avoid detached threads and unbounded queues.

## Module Boundaries

Each module owns its implementation and exposes a small public interface. Other modules must not reach into its internal files or data structures.

| Module | Responsibility | Must Not Own |
|---|---|---|
| Storage | WAL, memtable, SSTables, indexes, compaction | Replica selection or sockets |
| Cluster | Node identity, consistent hashing, membership, failure state | Record persistence |
| Replication | Replica requests, hints, read repair, convergence | CLI parsing |
| Coordinator | Read/write quorum decisions and consistency levels | Storage file formats |
| Networking | Protocol encoding, sockets, request transport | Quorum policy |
| Client | User commands and client-side request handling | Server internals |
| Deployment | Docker image and Compose configuration | Database behavior |
| Metrics and benchmarks | Measurement and workload generation | Correctness decisions |

Dependencies should point inward through interfaces. For example, the coordinator may call storage and transport interfaces, while storage must not depend on the coordinator.

Shared interfaces should be agreed on and merged before both owners build against them. A change to a shared contract must include the affected owner as a reviewer.

## Dependency Policy

Quorum DB implements its core database and distributed-systems behavior itself. Production dependencies should be rare and must not replace a concept the project intends to teach.

Allowed by default:

- C++ standard library
- Operating-system socket and file APIs behind project interfaces
- GoogleTest, pinned to an exact release
- CMake and CTest development tooling

Requires agreement from both contributors:

- Formatting, static-analysis, benchmark, or metrics libraries
- Compression and checksum libraries
- Any production dependency

Out of scope for the core implementation:

- Existing database or LSM-tree engines
- Distributed consensus, membership, or replication frameworks
- RPC or serialization frameworks that replace the custom binary protocol
- Large utility frameworks added for a small helper feature

Dependencies must be pinned, declared through CMake, documented with their purpose, and compatible with the project license. Do not commit generated dependency build output.

## Tests and Reviews

- Keep unit tests beside the module they validate in the test hierarchy.
- Name tests by behavior, such as `WalReplayIgnoresIncompleteFinalRecord`.
- Every bug fix must include a regression test.
- Keep tests deterministic and avoid long sleeps.
- Persistent-storage tests must use isolated temporary directories.
- Each pull request should stay within one owned module when practical.
- The other contributor reviews changes to shared interfaces and milestone integrations.

The detailed test layout, fixture naming, and integration-test organization belong in `docs/testing.md`.

## Definition of Ready for Implementation

Implementation can begin when the repository structure and test strategy are agreed on, the root CMake build exists, and both contributors can build and run one placeholder test with the documented commands.
