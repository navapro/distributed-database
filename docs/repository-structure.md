# Repository Structure

Quorum DB will use a small folder structure based on major concepts.

```text
distributed-database/
|-- CMakeLists.txt
|-- README.md
|-- planning.md
|-- docs/
|-- src/
|   |-- common/
|   |-- storage/
|   |-- cluster/
|   |-- replication/
|   |-- networking/
|   |-- client/
|   `-- node/
|-- tests/
|   |-- unit/
|   `-- integration/
|-- deployment/
`-- benchmark/
```

## Folder Purpose

| Folder | Purpose |
|---|---|
| `docs/` | Project designs and decisions. |
| `src/common/` | Small shared types such as `Record` and status results. |
| `src/storage/` | WAL, memtable, SSTables, Bloom filters, and compaction. |
| `src/cluster/` | Hash ring, node information, gossip, and failure detection. |
| `src/replication/` | Quorum coordination, hinted handoff, and read repair. |
| `src/networking/` | Binary protocol, TCP server, and TCP client transport. |
| `src/client/` | User-facing CLI commands. |
| `src/node/` | Database node startup and configuration. |
| `tests/unit/` | Small tests for one class or module. |
| `tests/integration/` | Basic tests that use multiple modules or nodes. |
| `deployment/` | Dockerfile and Docker Compose configuration. |
| `benchmark/` | Optional performance tool added after the database works. |

## Simple Rules

- Keep a header and its `.cpp` file in the same module folder.
- Add subfolders only when a module becomes difficult to navigate.
- Do not create separate folders for one or two files.
- A module may use another module only through its public headers.
- Keep executable startup files small; database behavior belongs in module code.
- Create each folder when its first real file is added. Empty placeholder files are not needed.

## First Files

The first implementation task should create only:

```text
src/common/record.h
src/common/record.cpp
tests/unit/record_test.cpp
```

The memtable and WAL folders can be added independently after the shared `Record` type is agreed on.
