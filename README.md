# Simple Async HTTP/1.0 Server

## Description

A lightweight, multi-threaded asynchronous HTTP/1.0 server, built using standalone asio.
It is capable of basic request parsing/handling and response writing with async I/O.

## Features

- Asynchronous networking using Asio
- Multi threaded request handling
- Treats all requests as HTTP/1.0 GET requests (doesn't validate)

## Requirements

- Asio 1.36.0 (standalone), pre-configured
- C++23 compiler
- CMake >= 3.25

## Building from Source

```bash
git clone https://github.com/sakchhu/http-server.git
cd http-server
cmake -S . -B build
cmake --build build
```

## Running the Server

Because the server resolves the document root relative to the working directory, it’s recommended to run it from the project root where the public/ directory is located.

```console
$ ./build/server <address> <port> <document-root> <threads>
Server listening on address:port
```

### Example Usage

Navigate to <http://127.0.0.1:8000>.

```console
$ ./build/server 127.0.0.1 8000 public 2
Server listening on 127.0.0.1:8000
```

## How It Works

1. Server binds a listener to requested endpoint
2. Server spawns N threads that each block until server receives a stop signal, or the connection was stopped.
3. Creates strands every time a connection is accepted, so all it's asynchronous operations are guaranteed to be sequentially invoked.
4. Each accepted connection is handled like so:
    - Read arbitrary data and try parsing it to a request object
    - If parser state is indeterminate, keep waiting for more input
    - If parser state is invalid, return a generic `400 Bad Request` Response
    - Handle the valid request and return an appropriate response
    - Close connection, and queue another async accept handler
5. The server therefore keeps accepting connections unless it is manually stopped.
