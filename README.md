# ZoltraakDB

A lightweight, high-performance single-threaded in-memory key-value database written in C++, inspired by Redis. ZoltraakDB provides a TCP server that accepts RESP (REdis Serialization Protocol) commands for fast and efficient data storage and retrieval.

## Overview

ZoltraakDB is a simple yet powerful database designed for high-throughput operations. It supports various data types, expiration policies (TTL), and uses efficient event-driven architecture with epoll for handling multiple concurrent connections.

## Features

- **In-Memory Key-Value Store**: Fast data access with support for multiple data types (integers, strings, and lists)
- **RESP Protocol Support**: Compatible with Redis Serialization Protocol for easy client integration
- **Expiration Management (TTL)**: Set automatic expiration times on keys with fine-grained TTL tracking
- **Concurrent Connection Handling**: Uses Linux epoll for efficient handling of multiple simultaneous client connections
- **Multi-Type Values**: Store integers, strings, and string vectors in a single database
- **Simple Command Set**: Essential operations like GET, SET, DELETE, and more

## Supported Commands

- **PING** - Check server connectivity
- **SET** `key value [ttl]` - Store a value with optional time-to-live in seconds
- **GET** `key` - Retrieve a value by key
- **DELETE** `key [key ...]` - Delete one or more keys
- **TTL** `key` - Get remaining time-to-live for a key in seconds
- **EXPIRE** `key seconds` - Set expiration time on an existing key

  ## Technical details
  Used an ***unordered_map<Zoltraak key,Zoltraak value>*** to store key value pairs and for faster lookups.
  
  Used the Linux ***epoll*** API to implement a non-blocking event loop.

## Building

### Requirements
- C++20 compatible compiler (g++ or clang++)
- Linux OS (for epoll support)

### Compilation

```bash
make
```
```bash
./zoltraak
```
