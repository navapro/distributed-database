# Storage Record Format

This document defines the first storage-level data model for Quorum DB. The same logical record should be used in the memtable, WAL, SSTable entries, replication messages, and read-repair comparisons.

## Logical Record

Each stored version of a key is represented as:

| Field | Type | Required | Description |
|---|---|---:|---|
| `key` | bytes/string | yes | Unique key used for lookup and sorted ordering. |
| `value` | bytes/string | no | Stored value. Empty when the record is a tombstone. |
| `timestamp` | unsigned 64-bit integer | yes | Monotonic version used for last-write-wins conflict resolution. |
| `tombstone` | boolean | yes | Marks a delete. Tombstones hide older values until compaction can safely remove them. |

## Rules

- Records are ordered by `key` inside memtables and SSTables.
- For the same key, the record with the highest `timestamp` wins.
- If two records have the same key and timestamp, a tombstone wins over a live value.
- A `GET` returns no value when the newest record for that key has `tombstone = true`.
- A `DELETE` is stored as a normal record with `tombstone = true`, not as immediate physical removal.
- Compaction may remove old overwritten values.
- Compaction may remove tombstones only after it is safe that older SSTables and replicas no longer need them.

## Example Records

Live value:

```text
key       = "user:123"
value     = "Naveen"
timestamp = 1042
tombstone = false
```

Delete marker:

```text
key       = "user:123"
value     = ""
timestamp = 1080
tombstone = true
```

In this example, the delete marker wins because `1080 > 1042`, so `GET user:123` should return not found.

## C++ Shape

The first implementation can use this shape:

```cpp
struct Record {
    std::string key;
    std::string value;
    std::uint64_t timestamp;
    bool tombstone;
};
```

Later, `key` and `value` can become byte buffers if Quorum DB needs arbitrary binary data.

## Last-Write-Wins Comparison

Use this comparison whenever replicas disagree:

1. Prefer the record with the larger `timestamp`.
2. If timestamps are equal, prefer the tombstone.
3. If both timestamp and tombstone are equal, use a deterministic tie-breaker such as node ID or lexicographic value.

The deterministic tie-breaker matters because all replicas must make the same decision.

## WAL Encoding Requirements

The WAL format should include:

- record length
- checksum
- key length
- value length
- timestamp
- tombstone flag
- key bytes
- value bytes

The WAL must append the record before the write is acknowledged. On restart, Quorum DB replays valid WAL records into the memtable.

## SSTable Encoding Requirements

Each SSTable entry should include:

- key length
- value length
- timestamp
- tombstone flag
- key bytes
- value bytes
- entry checksum or block checksum

SSTables must remain sorted by key. Bloom filters and sparse indexes should be built from the record keys.

## Test Cases

Minimum tests for this format:

- Put record returns the stored value.
- Newer timestamp overwrites older timestamp.
- Older timestamp does not overwrite newer timestamp.
- Tombstone hides older live value.
- Live value with newer timestamp can overwrite an older tombstone.
- Equal timestamp resolves deterministically.
- WAL replay restores records after restart.
- Corrupt encoded record is rejected using checksum.
