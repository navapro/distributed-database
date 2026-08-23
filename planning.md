# AtlasDB Project Plan

This plan splits the project between Naveen and Yashila so both people learn storage-engine internals, distributed systems, testing, and performance work. The assignments are intentionally mixed instead of giving one person only storage and the other only networking.

Priority labels:

- `P0`: must be done for the project to be credible
- `P1`: important once the core works
- `P2`: stretch or polish

Status labels:

- `Not Started`
- `In Progress`
- `Done`

## Phase 1: Storage Engine Foundation

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Define storage record format: key, value, timestamp, tombstone | Naveen | Not Started | data modeling, versioning |
| P0 | Implement WAL append before writes are acknowledged | Yashila | Not Started | durability, crash safety |
| P0 | Implement WAL replay on startup | Naveen | Not Started | recovery, idempotency |
| P0 | Implement sorted memtable | Yashila | Not Started | ordered indexes, write path |
| P0 | Flush memtable to immutable SSTable | Naveen | Not Started | LSM write path |
| P0 | Implement SSTable read path | Yashila | Not Started | sorted disk files, read amplification |
| P0 | Implement tombstones for deletes | Naveen | Not Started | delete semantics, compaction safety |
| P0 | Add SSTable checksums | Yashila | Not Started | corruption detection |
| P0 | Add storage unit tests | Naveen + Yashila | Not Started | test design, edge cases |

## Phase 2: Better LSM Internals

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Add Bloom filters to SSTables | Naveen | Not Started | probabilistic data structures |
| P0 | Add sparse index per SSTable | Yashila | Not Started | indexing, disk seeks |
| P0 | Implement size-tiered compaction | Naveen | Not Started | merge algorithms, write amplification |
| P1 | Add block cache | Yashila | Not Started | caching, locality |
| P1 | Add background flush worker | Naveen | Not Started | concurrency, producer/consumer |
| P1 | Add background compaction worker | Yashila | Not Started | scheduling, thread safety |
| P1 | Track storage metrics | Naveen + Yashila | Not Started | observability |

## Phase 3: In-Process Distributed Database

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Implement consistent hash ring | Naveen | Not Started | hashing, token ownership |
| P0 | Add virtual nodes | Yashila | Not Started | load balancing, rebalancing |
| P0 | Implement cluster node abstraction | Naveen | Not Started | architecture boundaries |
| P0 | Implement coordinator write path | Yashila | Not Started | leaderless replication |
| P0 | Implement coordinator read path | Naveen | Not Started | quorum reads |
| P0 | Support replication factor configuration | Yashila | Not Started | replica placement |
| P0 | Support consistency levels: `ONE`, `QUORUM`, `ALL` | Naveen | Not Started | tunable consistency |
| P0 | Implement last-write-wins conflict resolution | Yashila | Not Started | conflict resolution |
| P0 | Add integration tests for quorum behavior | Naveen + Yashila | Not Started | distributed correctness |

## Phase 4: Failure Handling and Repair

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Add manual node up/down simulation | Naveen | Not Started | failure modeling |
| P0 | Implement hinted handoff | Yashila | Not Started | availability, delayed repair |
| P0 | Replay hints when nodes recover | Naveen | Not Started | recovery workflows |
| P0 | Implement read repair | Yashila | Not Started | replica convergence |
| P1 | Implement anti-entropy repair command | Naveen | Not Started | background repair |
| P1 | Implement Merkle tree comparison | Yashila | Not Started | range hashing, efficient repair |
| P1 | Add repair metrics | Naveen + Yashila | Not Started | observability |

## Phase 5: Networking and CLI

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Design custom binary protocol | Naveen | Not Started | protocol design, serialization |
| P0 | Implement protocol encoder/decoder | Yashila | Not Started | binary formats |
| P0 | Build basic TCP node server | Naveen | Not Started | sockets, request handling |
| P0 | Build basic TCP client | Yashila | Not Started | client/server systems |
| P0 | Add CLI command: `atlas put` | Naveen | Not Started | user tooling |
| P0 | Add CLI command: `atlas get` | Yashila | Not Started | user tooling |
| P0 | Add CLI command: `atlas delete` | Naveen | Not Started | user tooling |
| P0 | Add CLI command: `atlasctl status` | Yashila | Not Started | cluster management |
| P1 | Add request timeouts | Naveen | Not Started | reliability |
| P1 | Add bounded request queue | Yashila | Not Started | backpressure |

## Phase 6: Gossip and Failure Detection

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P1 | Define gossip node state | Naveen | Not Started | membership metadata |
| P1 | Implement gossip merge rules | Yashila | Not Started | eventual convergence |
| P1 | Add periodic gossip exchange | Naveen | Not Started | distributed coordination |
| P1 | Implement phi accrual failure detector | Yashila | Not Started | adaptive failure detection |
| P1 | Connect gossip state to ring status | Naveen | Not Started | cluster visibility |
| P1 | Add failure detector tests | Naveen + Yashila | Not Started | timing-sensitive tests |

## Phase 7: Rebalancing and Streaming

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P1 | Compute token ranges owned by each node | Naveen | Not Started | consistent hashing ranges |
| P1 | Implement range scan from storage engine | Yashila | Not Started | ordered iteration |
| P1 | Implement range streaming between nodes | Naveen | Not Started | data movement |
| P1 | Add node join flow | Yashila | Not Started | rebalancing |
| P1 | Add node removal/decommission flow | Naveen | Not Started | cluster operations |
| P2 | Stream SSTable ranges instead of key batches | Yashila | Not Started | performance engineering |

## Phase 8: Testing, Simulation, and Chaos

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P0 | Add crash/restart tests for WAL | Naveen | Not Started | crash safety |
| P0 | Add failure tests for quorum availability | Yashila | Not Started | availability guarantees |
| P1 | Build randomized workload generator | Naveen | Not Started | property-style testing |
| P1 | Record operation history | Yashila | Not Started | correctness checking |
| P1 | Build deterministic simulator with seed | Naveen | Not Started | reproducible distributed bugs |
| P1 | Add simulated network partitions | Yashila | Not Started | partition tolerance |
| P2 | Build chaos testing executable | Naveen + Yashila | Not Started | fault injection |
| P2 | Add Jepsen-style history checker | Naveen + Yashila | Not Started | formal-ish validation |

## Phase 9: Benchmarking and Performance

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P1 | Build benchmark executable | Naveen | Not Started | measurement methodology |
| P1 | Report throughput | Yashila | Not Started | performance metrics |
| P1 | Report p50, p95, p99 latency | Naveen | Not Started | latency analysis |
| P1 | Benchmark consistency levels | Yashila | Not Started | consistency vs latency |
| P1 | Benchmark Bloom filter negative reads | Naveen | Not Started | storage optimization |
| P2 | Benchmark replication factor scaling | Yashila | Not Started | distributed performance |
| P2 | Profile with sanitizers/profilers | Naveen + Yashila | Not Started | debugging, optimization |

## Phase 10: Stretch Features

| Priority | Task | Owner | Status | Concepts |
|---|---|---|---|---|
| P2 | Paxos-based compare-and-set | Naveen + Yashila | Not Started | consensus |
| P2 | Vector clocks | Naveen | Not Started | causal metadata |
| P2 | Hybrid logical clocks | Yashila | Not Started | distributed time |
| P2 | Docker Compose local cluster | Naveen | Not Started | deployment |
| P2 | Prometheus metrics endpoint | Yashila | Not Started | observability |
| P2 | Simple cluster dashboard | Naveen + Yashila | Not Started | visualization |

## Learning Rotation

To make sure both people learn the important parts:

- Naveen starts with WAL replay, SSTables, coordinator reads, and consistent hashing.
- Yashila starts with WAL append, memtables, coordinator writes, and virtual nodes.
- Naveen owns the first version of custom protocol design; Yashila owns the encoder/decoder implementation.
- Yashila owns the first version of phi accrual failure detection; Naveen owns gossip state integration.
- Naveen owns benchmark structure; Yashila owns benchmark reporting.
- Pair on tests whenever the feature affects correctness or failure behavior.

## Weekly Working Style

Each week:

1. Pick 2-4 tasks total, not 10.
2. Each person owns at least one implementation task.
3. Each person reviews or tests the other person's work.
4. End the week by updating this file with status changes.
5. Write down one concept each person learned.

## First Four Weeks

### Week 1

- Naveen: storage record format, WAL replay test
- Yashila: WAL append, sorted memtable
- Pair: storage unit test structure

### Week 2

- Naveen: SSTable flush/write path
- Yashila: SSTable read path
- Pair: tombstone and restart tests

### Week 3

- Naveen: consistent hash ring
- Yashila: virtual nodes
- Pair: replica placement tests

### Week 4

- Naveen: coordinator read path with `QUORUM`
- Yashila: coordinator write path with `QUORUM`
- Pair: integration tests for node failure and recovery

## Definition of Done

A task is done only when:

- The code builds.
- Tests exist for the main behavior.
- Edge cases are documented or tested.
- The README or docs are updated if behavior changed.
- The other person has reviewed the work.

## Biggest Risks

- Trying to build networking before the storage engine is reliable.
- Claiming consistency guarantees that are not tested.
- Spending too much time on dashboards or deployment too early.
- Building many partial features instead of finishing the core path.
- Not writing crash and failure tests.

## Guiding Rule

Make the smallest version that teaches the real concept, test it under failure, then improve it.
