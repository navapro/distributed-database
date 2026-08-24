# Quorum DB Project Plan

This plan is organized for two people working concurrently with low merge conflict risk. Naveen and Yashila should usually work in different modules, with short integration checkpoints where both branches are merged and tested together.

Priority labels:

- `P0`: must be done for the project to be credible
- `P1`: important once the core works
- `P2`: stretch or polish

Status labels:

- `Not Started`
- `In Progress`
- `Blocked`
- `Done`

## Ownership Model

Each person owns a module area for a milestone. The other person can review, test, or pair, but should avoid editing the same files during that milestone unless it is an integration checkpoint.

| Area | Primary Owner | Reviewer | Main Files Later |
|---|---|---|---|
| Project docs and planning | Naveen | Yashila | `README.md`, `planning.md`, `docs/` |
| Storage record contract | Naveen | Yashila | `docs/storage-record-format.md`, later `record.*` |
| WAL and crash recovery | Yashila | Naveen | `storage/wal/` |
| Memtable and in-memory ordering | Naveen | Yashila | `storage/memtable/` |
| SSTable read/write format | Yashila | Naveen | `storage/sstable/` |
| Bloom filters and sparse indexes | Naveen | Yashila | `storage/bloom/`, `storage/sstable/` |
| Compaction | Yashila | Naveen | `storage/compaction/` |
| Hash ring and virtual nodes | Naveen | Yashila | `cluster/hashing/` |
| Coordinator and quorum logic | Yashila | Naveen | `coordinator/`, `replication/` |
| Failure handling and repair | Naveen | Yashila | `repair/`, `replication/` |
| TCP protocol and server | Naveen | Yashila | `networking/` |
| CLI tools | Yashila | Naveen | `client/` |
| Docker and local cluster deployment | Yashila | Naveen | `deployment/`, `Dockerfile`, `compose.yaml` |
| Basic end-to-end testing | Naveen | Yashila | `tests/storage_integration/`, `tests/cluster_integration/` |
| Benchmarks and metrics | Yashila | Naveen | `benchmark/`, `metrics/` |

## Concurrency Rules

- Only one person edits a module's implementation files at a time.
- Shared headers/interfaces should be changed in small PRs before implementation work depends on them.
- If both people need the same interface, write the interface first, merge it, then split implementation.
- Pair on design and tests, but avoid both editing the same source file.
- Integration happens at planned checkpoints, not continuously in the middle of feature work.
- Each task should include tests in the same module when possible.

## Milestone 0: Project Contracts

Goal: define stable contracts so later work can happen in parallel.

| Priority | Task | Owner | Status | Can Run Concurrently With | Output |
|---|---|---|---|---|---|
| P0 | Define storage record format: key, value, timestamp, tombstone | Naveen | Done | WAL design, memtable design | `docs/storage-record-format.md` |
| P0 | Define repository folder structure | Yashila | Done | storage docs | `docs/repository-structure.md` |
| P0 | Define coding conventions and build approach | Naveen | Done | test strategy | `docs/development.md` |
| P0 | Define test strategy and naming | Yashila | Done | build approach | `docs/testing.md` |
| P0 | Create minimal CMake and GoogleTest setup | Yashila | Done | repository structure | `CMakeLists.txt`, `tests/` |

Milestone status: Done

Integration checkpoint:

- [x] Agree on folder structure.
- [x] Agree on record interface.
- [x] Agree on how tests will be run.
- [x] Add the root build and test setup.

## Milestone 1: Single-Node Storage Engine

Goal: build a durable local key-value engine before distributed features.

### Parallel Track A: Memtable and Record Semantics

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Define memtable interface and behavior | Done | storage record format | ordered in-memory storage |
| P0 | Implement `Record` type and last-write-wins comparison | Done | storage record format | versioning |
| P0 | Implement sorted memtable interface | Done | `Record` type | ordered maps/skiplists |
| P0 | Implement `PUT`, `GET`, and tombstone behavior in memtable | Done | memtable interface | write path |
| P0 | Add memtable unit tests | Done | memtable behavior | test design |

### Parallel Track B: WAL

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Design binary WAL entry format | Not Started | storage record format | durability |
| P0 | Implement WAL append | Not Started | WAL format | file I/O |
| P0 | Implement WAL iterator/replay reader | Not Started | WAL append | crash recovery |
| P0 | Add WAL checksum validation | Not Started | WAL reader | corruption detection |
| P0 | Add WAL unit tests | Not Started | WAL implementation | recovery tests |

Integration checkpoint:

- Connect memtable writes to WAL append.
- Verify acknowledged writes survive restart.
- Run memtable and WAL tests together.

### Parallel Track C: SSTable Writer

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Design SSTable entry format | Not Started | storage record format | disk layout |
| P0 | Write sorted memtable records to SSTable | Not Started | memtable iterator | immutable files |
| P0 | Add SSTable checksums | Not Started | SSTable writer | crash safety |
| P0 | Add SSTable writer tests | Not Started | SSTable writer | persistence |

### Parallel Track D: SSTable Reader

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Implement SSTable sequential reader | Not Started | SSTable format | parsing |
| P0 | Implement point lookup from SSTable | Not Started | sequential reader | read path |
| P0 | Add tombstone handling in SSTable reads | Not Started | point lookup | delete semantics |
| P0 | Add SSTable reader tests | Not Started | SSTable reader | edge cases |

Integration checkpoint:

- Flush memtable to SSTable.
- Read values back after process restart.
- Confirm tombstones survive flush and restart.

## Milestone 2: LSM Read Path and Compaction

Goal: make storage efficient enough to support distributed reads.

### Parallel Track A: Lookup Acceleration

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Add Bloom filter data structure | Not Started | SSTable keys | probabilistic filtering |
| P0 | Persist Bloom filter metadata per SSTable | Not Started | Bloom filter | storage metadata |
| P0 | Add sparse index format | Not Started | SSTable writer | indexing |
| P0 | Use Bloom filter and sparse index in point lookup | Not Started | metadata formats | read amplification |
| P0 | Benchmark negative reads with and without Bloom filter | Not Started | lookup acceleration | measurement |

### Parallel Track B: Compaction

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Implement merge iterator across SSTables | Not Started | SSTable reader | merge algorithms |
| P0 | Implement size-tiered compaction | Not Started | merge iterator | write amplification |
| P0 | Eliminate overwritten records during compaction | Not Started | last-write-wins | storage cleanup |
| P0 | Preserve tombstones safely during compaction | Not Started | tombstone rules | delete safety |
| P0 | Add compaction tests | Not Started | compaction | correctness |

Integration checkpoint:

- Read path checks memtable, then newest SSTables.
- Compaction output is readable by the normal SSTable reader.
- Negative reads use Bloom filters.

## Milestone 3: In-Process Distributed Core

Goal: model a leaderless cluster in one process before TCP networking.

### Parallel Track A: Ring and Membership Model

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Implement consistent hash ring | Not Started | none | token ownership |
| P0 | Add virtual nodes | Not Started | hash ring | load distribution |
| P0 | Compute replica preference list for a key | Not Started | virtual nodes | replica placement |
| P0 | Add ring unit tests | Not Started | preference list | deterministic hashing |

### Parallel Track B: Quorum Coordinator

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Define coordinator request/response types | Not Started | storage API | boundaries |
| P0 | Implement write coordinator for `ONE`, `QUORUM`, `ALL` | Not Started | preference list interface | leaderless writes |
| P0 | Implement read coordinator for `ONE`, `QUORUM`, `ALL` | Not Started | storage API | quorum reads |
| P0 | Add last-write-wins conflict resolution in coordinator | Not Started | `Record` comparison | consistency |
| P0 | Add coordinator unit tests with fake replicas | Not Started | coordinator | test doubles |

Integration checkpoint:

- Coordinator uses the real hash ring.
- In-process cluster can write and read with replication factor 3.
- Quorum tests pass with fake node failures.

## Milestone 4: Failure Handling and Repair

Goal: keep the system available during replica failures and converge after recovery.

### Parallel Track A: Failure State and Hints

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Add node health state to in-process cluster | Not Started | cluster model | failure modeling |
| P0 | Store hinted handoff records for down replicas | Not Started | write coordinator | availability |
| P0 | Replay hints after node recovery | Not Started | hint storage | recovery |
| P0 | Add hinted handoff tests | Not Started | hint replay | fault tolerance |

### Parallel Track B: Read Repair

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Detect stale replica responses during reads | Not Started | read coordinator | convergence |
| P0 | Write repaired record back to stale replicas | Not Started | stale detection | repair |
| P0 | Add read-repair metrics | Not Started | repair path | observability |
| P0 | Add read-repair tests | Not Started | repair path | distributed correctness |

Integration checkpoint:

- A recovered node receives missed writes.
- A stale live replica is repaired by a read.
- Quorum behavior remains correct while one replica is down.

## Milestone 5: TCP Protocol and CLI

Goal: move from in-process simulation to multiple processes.

### Parallel Track A: Server and Protocol

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Design binary protocol header | Not Started | request types | serialization |
| P0 | Implement protocol encoder/decoder | Not Started | protocol header | binary formats |
| P0 | Build basic TCP node server | Not Started | coordinator API | sockets |
| P1 | Add request IDs and timeouts | Not Started | TCP server | reliability |
| P1 | Add bounded request queue | Not Started | TCP server | backpressure |

### Parallel Track B: Client and Control CLI

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Build TCP client library | Not Started | protocol encoder/decoder | client/server |
| P0 | Add `quorumdb put` | Not Started | client library | CLI design |
| P0 | Add `quorumdb get` | Not Started | client library | CLI design |
| P0 | Add `quorumdb delete` | Not Started | client library | CLI design |
| P0 | Add `quorumctl status` | Not Started | server status endpoint | operations |
| P1 | Add `quorumctl ring` | Not Started | ring status endpoint | observability |

### Parallel Track C: Docker Deployment

Owner: Yashila

This track uses separate deployment files, so it can proceed while Naveen works in `networking/`.

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Create a Docker image for one Quorum DB node | Not Started | runnable node process | containerization |
| P0 | Create a three-node Docker Compose cluster | Not Started | Docker image | local orchestration |
| P0 | Give each node an independent persistent volume | Not Started | Compose cluster | durable container storage |
| P0 | Configure node ID, ports, seeds, and replication through environment variables | Not Started | node configuration | deployment configuration |
| P0 | Document cluster start, stop, restart, and log commands | Not Started | Compose cluster | local operations |
| P0 | Test node failure and recovery using container stop/restart | Not Started | quorum integration | failure testing |

Integration checkpoint:

- Start three node containers with Docker Compose.
- Write through one node and read through another.
- Stop and restart one container without losing acknowledged data.
- Preserve each node's data in its own volume.
- CLI can show node status and ring ownership.

## Milestone 6: Gossip and Real Failure Detection

Goal: replace manual failure toggles with cluster membership and adaptive suspicion.

### Parallel Track A: Gossip

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Define gossip node state | Not Started | node identity | membership metadata |
| P1 | Implement gossip merge rules | Not Started | gossip state | eventual convergence |
| P1 | Add periodic gossip exchange | Not Started | TCP internode messages | distributed coordination |
| P1 | Add gossip tests | Not Started | merge rules | convergence |

### Parallel Track B: Phi Accrual Failure Detector

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Track heartbeat arrival intervals | Not Started | time abstraction | failure detection |
| P1 | Compute phi suspicion score | Not Started | heartbeat history | statistics |
| P1 | Mark nodes suspected/unavailable based on phi | Not Started | phi score | adaptive health |
| P1 | Add basic failure detector tests | Not Started | detector | timing tests |

Integration checkpoint:

- Gossip carries heartbeat state.
- Failure detector updates node health used by coordinator.
- `quorumctl status` shows healthy/suspected/down.

## Milestone 7: Repair, Rebalancing, and Streaming

Goal: support long-running clusters where data moves and divergence is repaired efficiently.

### Parallel Track A: Repair

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Build Merkle tree for key ranges | Not Started | storage range scan | anti-entropy |
| P1 | Compare Merkle trees between replicas | Not Started | Merkle tree | efficient diffing |
| P1 | Repair differing ranges | Not Started | range streaming | convergence |
| P1 | Add manual `quorumctl repair` command | Not Started | repair API | operations |

### Parallel Track B: Rebalancing and Streaming

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Compute token ranges owned by each node | Not Started | hash ring | range ownership |
| P1 | Implement storage range scan | Not Started | SSTable iteration | ordered iteration |
| P1 | Stream key ranges between nodes | Not Started | networking | data movement |
| P1 | Add node join flow | Not Started | range streaming | rebalancing |
| P1 | Add node decommission flow | Not Started | range streaming | cluster operations |

Integration checkpoint:

- Adding a node transfers the ranges it should own.
- Removing a node preserves availability.
- Anti-entropy repair transfers only differing ranges.

## Milestone 8: Basic End-to-End Testing

Goal: check the main storage and cluster behavior without building a custom testing system.

### Parallel Track A: Storage and Restart Tests

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Test `PUT`, `GET`, and `DELETE` on one node | Not Started | node executable | basic behavior |
| P0 | Restart the node and verify WAL recovery | Not Started | WAL replay | durability |
| P0 | Verify a deleted key stays deleted after restart | Not Started | tombstones | recovery |

### Parallel Track B: Three-Node Quorum Tests

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P0 | Start three nodes with Docker Compose | Not Started | Compose cluster | deployment |
| P0 | Test `ONE`, `QUORUM`, and `ALL` | Not Started | coordinator | consistency levels |
| P0 | Stop one node and verify QUORUM still works | Not Started | failure handling | availability |
| P0 | Restart the node and verify missed data is recovered | Not Started | hinted handoff | recovery |

Integration checkpoint:

- All seven checks pass using normal commands and GoogleTest where useful.
- No simulator, history checker, or chaos framework is required.

## Milestone 9: Benchmarks and Observability

Goal: report real performance numbers and understand tradeoffs.

### Parallel Track A: Benchmark Driver

Owner: Yashila

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Build benchmark executable | Not Started | CLI/client API | measurement |
| P1 | Add configurable clients and operations | Not Started | benchmark executable | workload design |
| P1 | Report throughput | Not Started | workload driver | performance |
| P1 | Report p50, p95, p99 latency | Not Started | latency recorder | tail latency |

### Parallel Track B: Metrics and Experiments

Owner: Naveen

| Priority | Task | Status | Depends On | Concepts |
|---|---|---|---|---|
| P1 | Add request counters | Not Started | coordinator | observability |
| P1 | Add storage metrics | Not Started | storage engine | observability |
| P1 | Add replication metrics | Not Started | coordinator/repair | observability |
| P1 | Benchmark consistency levels | Not Started | benchmark driver | tradeoff analysis |
| P1 | Benchmark Bloom filter negative reads | Not Started | storage metrics | storage optimization |

Integration checkpoint:

- Benchmark output includes throughput and latency percentiles.
- Metrics explain at least one performance result.
- Results are documented in `docs/benchmarks.md`.

## Milestone 10: Stretch Features

Only start these after the P0 system works end to end.

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P2 | Paxos-based compare-and-set | Naveen + Yashila | Not Started | consensus |
| P2 | Vector clocks | Naveen | Not Started | causal metadata |
| P2 | Hybrid logical clocks | Yashila | Not Started | distributed time |
| P2 | Kubernetes deployment manifests | Naveen | Not Started | orchestration |
| P2 | Prometheus metrics endpoint | Yashila | Not Started | observability |
| P2 | Simple cluster dashboard | Naveen + Yashila | Not Started | visualization |

## First Four Weeks

### Week 1: Contracts and Independent Storage Modules

- Naveen: finish development docs, implement or prepare memtable interface.
- Yashila: define repository folder structure, design WAL format.
- Integration: agree on `Record` API and test layout.

### Week 2: WAL and Memtable

- Naveen: implement memtable behavior and tests.
- Yashila: implement WAL append/replay and tests.
- Integration: connect WAL plus memtable write path.

### Week 3: SSTable Split

- Naveen: implement SSTable reader and point lookup tests.
- Yashila: implement SSTable writer and checksum tests.
- Integration: flush memtable to SSTable and read it back.

### Week 4: Hashing and Coordinator

- Naveen: implement hash ring, virtual nodes, replica preference tests.
- Yashila: implement coordinator tests using fake replicas.
- Integration: coordinator uses real preference lists.

## Definition of Done

A task is done only when:

- The code builds.
- Tests exist for the main behavior.
- Edge cases are documented or tested.
- Public behavior is documented if it changed.
- The other person has reviewed the work.
- Integration checkpoint passes before the next milestone starts.

## Biggest Risks

- Both people editing the same files at the same time.
- Starting networking before storage and coordinator interfaces are stable.
- Claiming consistency guarantees that are not tested.
- Spending too much time on dashboards or deployment too early.
- Building many partial features instead of finishing the core path.
- Not writing crash and failure tests.

## Guiding Rule

Own separate modules, merge at planned checkpoints, and make the smallest version that teaches the real concept before adding complexity.
