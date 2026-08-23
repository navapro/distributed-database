# Memtable Design

The memtable keeps recent records in memory before they are written to an SSTable.

## Data Structure

Use `std::map<std::string, Record>`.

This is a good first version because:

- each key has one current record
- lookup is efficient
- records stay sorted by key
- sorted records can be written directly to an SSTable

We do not need a skip list or custom tree for the first version.

## Core Operations

The memtable needs these operations:

| Operation | Behavior |
|---|---|
| `put(record)` | Insert the record, or replace the existing record if the new one wins. |
| `get(key)` | Return the current `Record` for the key, or no record if the key is absent. |
| `records()` | Return all records in sorted key order for flushing. |
| `size_bytes()` | Return the approximate memory used by keys and values. |
| `clear()` | Remove all records after a successful flush. |

`get` returns the complete `Record`, including its tombstone flag. The storage engine decides whether the client should receive a value or `not found`.

## Write Rules

When `put(record)` is called:

1. If the key is absent, insert the record.
2. If the key exists, keep the record with the larger timestamp.
3. If timestamps are equal, prefer a tombstone over a live value.
4. If both have the same timestamp and tombstone flag, prefer the record with the lexicographically larger value. This makes the result deterministic.

An older record must never replace a newer record.

## Delete Rules

A delete does not remove a key from the map. It inserts a record with:

```text
value = ""
tombstone = true
```

The tombstone must remain in the memtable and later be written to the SSTable. This prevents older values from becoming visible again.

## Memory Tracking

Track an approximate byte count:

```text
key length + value length + fixed Record fields
```

When a record is replaced, subtract the old record's estimated size and add the new record's estimated size.

Exact allocator memory usage is not required. The count only decides when the memtable is large enough to flush.

## Flush Behavior

When the memtable reaches its configured size limit:

1. Stop adding records to that memtable.
2. Iterate over its records in key order.
3. Write the records to a new SSTable.
4. Clear the memtable only after the SSTable is written successfully.

If the SSTable write fails, keep the memtable records so the flush can be retried.

## Write Path

For each client write:

1. Append the record to the WAL.
2. Wait for the WAL append to succeed.
3. Put the record into the memtable.
4. Acknowledge the write.

Writing to the WAL first allows the memtable to be rebuilt after a crash.

## First-Version Limits

Keep the first implementation simple:

- one active memtable
- single-threaded access
- configurable size limit
- no background flushing
- no skip list
- no memory allocator optimization

Thread safety and background flushing can be added after the basic storage engine works.

## Minimum Tests

- Insert and retrieve a record, and return no record for a missing key.
- Keep records sorted by key.
- Keep the newest record for a key.
- Prefer a tombstone when timestamps are equal.
- Return tombstones from `get`.
- Update the approximate size when a record is replaced.
