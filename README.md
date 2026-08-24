# Quorum DB

A Cassandra/Dynamo-inspired, leaderless distributed key-value database written from scratch in C++.

Quorum DB is meant to demonstrate database internals and distributed-systems engineering without trying to clone all of Cassandra. The project should be small enough to finish, but deep enough to show storage durability, replication, failure handling, repair, and correctness testing.

## Project Goal

Build a fault-tolerant key-value database where any healthy node can accept client requests and coordinate reads or writes across replicas.

Core themes:

- LSM-tree storage engine internals
- Leaderless replication
- Consistent hashing and virtual nodes
- Tunable consistency
- Failure detection and recovery
- Read repair and hinted handoff
- Simple unit and end-to-end testing
- Benchmarking and performance analysis

Project documents:

- [Project plan](planning.md)
- [Development guide](docs/development.md)
- [Repository structure](docs/repository-structure.md)
- [Testing guide](docs/testing.md)
- [Storage record format](docs/storage-record-format.md)
- [Memtable design](docs/memtable-design.md)
- [SSTable format](docs/sstable-format.md)

## Must Have

These are the minimum features needed for the project to feel complete and credible.

### Storage Engine

- Write-ahead log before acknowledging writes
- Storage record format: [docs/storage-record-format.md](docs/storage-record-format.md)
- WAL replay on restart
- Sorted memtable
- Immutable flushed SSTables
- Tombstones for deletes
- SSTable checksums or corruption detection
- Basic sparse index
- Bloom filter for negative lookups
- Size-tiered compaction
- Unit tests for WAL, SSTables, tombstones, Bloom filters, and compaction

### Distributed Core

- Leaderless design where any node can act as coordinator
- Consistent hash ring
- Virtual nodes
- Configurable replication factor
- `GET`, `PUT`, and `DELETE`
- Tunable consistency levels: `ONE`, `QUORUM`, `ALL`
- Last-write-wins conflict resolution
- Read repair
- Hinted handoff
- Basic node failure and recovery flow
- Integration tests for quorum reads/writes and replica recovery

### Networking and CLI

- Custom binary TCP protocol
- Client request IDs
- Basic request timeouts
- CLI commands:
  - `quorumdb put <key> <value>`
  - `quorumdb get <key>`
  - `quorumdb delete <key>`
  - `quorumctl status`
  - `quorumctl ring`

### Local Deployment

- Docker image for a Quorum DB node
- Docker Compose configuration for a three-node local cluster
- Separate persistent volume for each node
- Environment-based node identity, ports, seed nodes, and replication settings
- Container-based integration tests for startup, shutdown, restart, and node failure

### Testing and Correctness

- Focused unit tests for each core module
- A few end-to-end tests for replication and recovery
- Crash/restart test for WAL durability
- Documented consistency guarantees

### Observability

- Basic internal metrics:
  - request counts
  - read/write/delete counts
  - SSTable count
  - WAL size
  - pending hints
  - live/dead nodes

## Should Have

These features make the project much stronger, but the database can still be considered successful without every one of them.

### Storage Improvements

- Block cache
- More realistic SSTable block format
- Background flushing
- Background compaction
- Tombstone garbage collection after a safety window
- Compaction metrics:
  - write amplification
  - read amplification
  - disk amplification

### Cluster Management

- Gossip membership protocol
- Phi accrual failure detector
- Node join
- Node removal
- Manual repair command
- Range ownership reporting

### Repair and Rebalancing

- Merkle-tree anti-entropy repair
- Range streaming between nodes
- Rebalancing after adding a node
- Decommission flow for removing a node

### Performance Tools

- Benchmark executable:
  - configurable node count
  - configurable clients
  - read/write ratio
  - consistency level
- Latency reporting:
  - p50
  - p95
  - p99
- Bloom-filter benchmark for negative reads

## Stretch Goals

These are impressive extensions, but they should only be attempted after the must-have system works reliably.

### Advanced Consistency

- Paxos-based compare-and-set
- Vector clocks
- Hybrid logical clocks
- CRDT experiment

### Advanced Networking

- Non-blocking sockets
- `epoll` on Linux
- Thread pools
- Connection pooling
- Backpressure and overload responses
- Batched writes
- Buffer pooling

### Deployment and Dashboard

- Kubernetes manifests
- Simple cluster dashboard
- Prometheus metrics endpoint
- Grafana dashboard

## Not a Priority

These are intentionally out of scope unless everything else is already solid.

- SQL
- Cassandra query language compatibility
- Authentication and user accounts
- Secondary indexes
- Joins
- Full-text search
- Complex frontend work

## Suggested Milestones

### Milestone 1: Storage Engine

Build the single-node LSM engine first.

Deliverables:

- WAL append and replay
- Memtable
- SSTable write/read path
- Tombstones
- Checksums
- Bloom filters
- Compaction
- Storage unit tests

Success criteria:

- Acknowledged writes survive restart.
- Deleted keys stay deleted after restart and compaction.
- Corrupt SSTables are detected.
- Negative reads skip SSTables using Bloom filters.

### Milestone 2: In-Process Distributed Model

Before real networking, model a cluster inside one process.

Deliverables:

- Nodes
- Coordinator logic
- Consistent hash ring
- Virtual nodes
- Replication factor
- `ONE`, `QUORUM`, `ALL`
- Hinted handoff
- Read repair

Success criteria:

- QUORUM writes succeed when enough replicas are alive.
- ALL writes fail when any required replica is unavailable.
- Failed replicas receive missed writes after recovery.
- Stale replicas are repaired after reads.

### Milestone 3: TCP Protocol and CLI

Add real process boundaries.

Deliverables:

- Binary request/response protocol
- Node server
- Client CLI
- `quorumctl status`
- Basic timeouts

Success criteria:

- Multiple node processes can run locally.
- A client can write through any node.
- Reads return the latest quorum-resolved value.

### Milestone 4: Failure Detection and Gossip

Make nodes discover and judge each other.

Deliverables:

- Gossip state exchange
- Phi accrual failure detector
- Membership state
- Ring status command

Success criteria:

- Nodes converge on cluster membership.
- Failed nodes become unavailable without manual marking.
- Recovered nodes rejoin and receive hints or repair.

### Milestone 5: Repair, Rebalancing, and Benchmarks

Make the system more realistic under change.

Deliverables:

- Merkle-tree repair
- Range streaming
- Add-node flow
- Remove-node flow
- Benchmark tool

Success criteria:

- Replica divergence is repaired without copying all data.
- Adding a node moves only the necessary ranges.
- Benchmarks report real throughput and latency numbers.

### Milestone 6: Basic End-to-End Testing

Check that the main database flow works across three containers.

Deliverables:

- Basic `PUT`, `GET`, and `DELETE` test
- WAL restart test
- `ONE`, `QUORUM`, and `ALL` test
- One-node stop and restart test

Success criteria:

- Acknowledged writes survive restart.
- QUORUM works while one of three nodes is down.
- A restarted node can recover missed data.

## Guarantees to Document

Quorum DB should be precise about what it guarantees.

- Durability: acknowledged writes are persisted through the WAL under the configured durability mode.
- Replication: each key maps to `N` replicas through the token ring.
- Consistency: operations wait for `ONE`, `QUORUM`, or `ALL` responses depending on the request.
- Conflict resolution: concurrent versions are resolved using last-write-wins.
- Availability: operations continue during failures when enough replicas remain alive for the requested consistency level.
- Repair: divergence is repaired through hinted handoff, read repair, and eventually anti-entropy repair.

## Recommended Build Order

1. Storage engine
2. In-process cluster simulation
3. Tunable consistency
4. Hinted handoff and read repair
5. Binary protocol
6. CLI
7. Gossip and failure detection
8. Repair and streaming
9. Benchmarks
10. Basic end-to-end tests

This order keeps the project honest: first make data durable, then make it replicated, then make it survive failures, then make it fast.
