# Testing Guide

Quorum DB will use simple tests that are easy to read and run.

## Tools

- GoogleTest for writing tests
- CTest for running tests through CMake

No custom test framework, simulator, or chaos tool is required.

## Test Folders

```text
tests/
|-- unit/
`-- integration/
```

- Unit tests check one type or module.
- Integration tests check that a few modules or nodes work together.

## Naming

- Test files use `<feature>_test.cpp`, such as `record_test.cpp`.
- Test suites use the class or module name, such as `RecordTest`.
- Test names describe the expected behavior, such as `NewerTimestampWins`.

Example:

```cpp
TEST(RecordTest, NewerTimestampWins) {
    // Arrange, act, and check the result.
}
```

## What to Test

For each small feature, test:

1. The normal case.
2. One important failure or edge case.

Do not test private implementation details.

The required integration checks are:

- An acknowledged write survives restart.
- `PUT`, `GET`, and `DELETE` work across the main storage path.
- A three-node cluster supports `ONE`, `QUORUM`, and `ALL`.
- QUORUM still works when one of three nodes is stopped.
- A restarted node recovers missed data.

## Running Tests

```sh
cmake -S . -B build -DQUORUMDB_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Test Rules

- Keep each test short and focused.
- Use temporary directories for WAL and SSTable tests.
- Do not depend on a particular test execution order.
- Avoid long sleeps and external services.
- Add a test when fixing a bug.
- More advanced testing can wait until the core project is complete.

A feature is sufficiently tested when its normal behavior and most important failure case pass consistently.
