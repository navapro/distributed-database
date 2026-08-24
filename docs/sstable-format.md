# SSTable Format

The first SSTable format is a single binary file containing records sorted by key.

## File Layout

The file starts with four bytes:

```text
QDB1
```

This identifies a Quorum DB SSTable and its format version.

The rest of the file contains entries until the end of the file.

## Entry Layout

| Field | Size |
|---|---:|
| Key length | 4 bytes |
| Value length | 4 bytes |
| Timestamp | 8 bytes |
| Tombstone | 1 byte |
| Key | Key length bytes |
| Value | Value length bytes |
| Checksum | 4 bytes |

All integers use little-endian byte order. Key and value lengths are limited to 16 MiB each.

The checksum uses 32-bit FNV-1a over every entry field except the checksum itself. A reader rejects the file if an entry is incomplete or its checksum does not match.

## Rules

- Entries must be sorted by key.
- Each key appears at most once in one SSTable.
- Tombstones are stored like normal records with an empty value.
- A missing key and a corrupt file are different results.
- The first version uses sequential reading; indexes can be added later.
