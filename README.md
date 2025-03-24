# C++ HTTP Server Library

A lightweight, event-driven HTTP server library written in C++ for Windows. This library provides a simple yet flexible framework for building web servers with support for routing, middleware, and JSON parsing. It leverages Winsock2 and IOCP (I/O Completion Ports) for asynchronous, non-blocking I/O, and employs a worker thread pool to efficiently handle many simultaneous connections.

## Table of Contents

- [C++ HTTP Server Library](#c-http-server-library)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Requirements](#requirements)
  - [Installation and Compilation](#installation-and-compilation)
  - [Usage](#usage)
    - [Initializing the Server](#initializing-the-server)
    - [Routing and HTTP Methods](#routing-and-http-methods)
    - [Middleware Support](#middleware-support)
    - [JSON Parsing and Response](#json-parsing-and-response)
  - [Architecture and Design](#architecture-and-design)
    - [Event-Driven Asynchronous I/O with IOCP](#event-driven-asynchronous-io-with-iocp)
    - [Worker Thread Pool](#worker-thread-pool)
    - [Connection State and Keep-Alive](#connection-state-and-keep-alive)
  - [API Reference](#api-reference)
    - [HttpServer Class](#httpserver-class)
    - [Request (Req) Class](#request-req-class)
    - [Response (Res) Class](#response-res-class)
    - [JSON Handling](#json-handling)
    - [Utilities](#utilities)
  - [License](#license)

## Features

- **Event-Driven, Asynchronous I/O:**  
  Utilizes Windows IOCP for non-blocking network I/O, enabling high scalability and efficient resource usage.
  
- **Worker Thread Pool:**  
  Processes I/O completions using a pool of worker threads, eliminating the need for a thread-per-connection model.
  
- **Routing:**  
  Supports HTTP methods such as GET, POST, PUT, PATCH, and DELETE with route-specific middleware.
  
- **Middleware Support:**  
  Chain middleware functions to process requests (e.g., JSON body parsing, URLencoded data) before reaching the route handler.
  
- **JSON Parsing and Stringifying:**  
  Convert between JSON strings and in-memory objects using modern C++ features (`std::variant`).  
  (You can also integrate faster libraries like rapidjson if desired.)
  
- **Query Parameter Parsing:**  
  Automatically extract query parameters from the request URL.
  
- **Keep-Alive and Connection Management:**  
  Supports both persistent (keep-alive) and non-persistent connections, with proper connection state management and cleanup.

## Requirements

- **Operating System:** Windows (using Winsock2 and IOCP for networking)
- **Compiler:** C++17 compliant compiler
- **Libraries:** Standard C++ libraries; Winsock2 (`ws2_32.lib`)

## Installation and Compilation

1. **Clone or Download the Repository** containing the following key files:
   - `httpserver.h` and `httpserver.cpp`
   - `json.h` and `json.cpp`
   - `request.h`
   - `response.h` and `response.cpp`
   - `middlewares.h`
   - `utils.h` and `utils.cpp`
   - `main.cpp`

2. **Compile the Library**  
   When using Visual Studio or another Windows C++ compiler, ensure that you link against `ws2_32.lib`. For example, if using the command line with `cl`:

   ```bash
   cl /EHsc httpserver.cpp json.cpp response.cpp utils.cpp main.cpp /link ws2_32.lib
   ```

   Replace `main.cpp` with your application source file if needed.

## Usage

### Initializing the Server

Create an instance of the `HttpServer` class and initialize it by specifying the address family, socket type, protocol, and port number. For example:

```cpp
#include "httpserver.h"

int main() {
  HttpServer server;
  // Initialize server for IPv4, TCP, and port 8080
  int serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8080);

  // The server starts listening asynchronously using IOCP and a worker thread pool.
  server.serverListen(serverSocket);
  
  // Main thread can perform other tasks or wait
  getchar();

  return 0;
}
```

### Routing and HTTP Methods

Define routes using the provided methods on the `HttpServer` class. Each route takes:

- A **path** (e.g., `"/hello"`)
- An optional vector of middleware functions
- A **handler** function that receives a request (`Req`) and response (`Res`) object.

Example of defining a GET route:

```cpp
#include "httpserver.h"
#include "request.h"
#include "response.h"

int main() {
  HttpServer server;
  server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8080);

  // Define a simple GET route
  server.Get("/hello", {},
    [](Req &req, Res &res) {
      res.send("Hello, world!")->status(200);
    }
  );

  server.serverListen(/*server socket returned from initServer*/);
  
  getchar();
  return 0;
}
```

### Middleware Support

Middleware functions allow you to preprocess requests before reaching the route handler. The library supports global middleware (applied to every request) using the `use()` method, and route-specific middleware as part of the route registration.

Example using a JSON body parser middleware:

```cpp
#include "httpserver.h"
#include "middlewares.h"

int main() {
  HttpServer server;
  SOCKET serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8080);

  // Apply global middleware to parse JSON bodies
  server.use(JsonBodyParser);

  // Define a POST route that expects JSON input
  server.Post("/data", {},
    [](Req &req, Res &res) {
      std::string responseText = "Received JSON: " + req.body.stringify();
      res.send(responseText)->status(200);
    }
  );

  server.serverListen(serverSocket);
  
  getchar();
  return 0;
}
```

### JSON Parsing and Response

The library provides a `JSONValue` class for representing JSON data. You can parse a JSON string into a `JSONValue` using `JSONParser` and also serialize a `JSONValue` back into a string with the `stringify()` method. The `Res` class supports a helper function `json()` to send JSON responses with appropriate headers.

Example sending a JSON response:

```cpp
#include "response.h"
#include "json.h"

void sendJsonResponse(Res &res) {
  JSONValue::Object obj;
  obj["message"] = JSONValue("Hello, JSON!");
  JSONValue jsonResponse(obj);
  res.json(jsonResponse)->status(200);
}
```

## Architecture and Design

### Event-Driven Asynchronous I/O with IOCP

- **IOCP-Based I/O:**  
  The library uses Windows I/O Completion Ports (IOCP) to perform all network operations asynchronously. Instead of blocking on `recv()` or `send()`, IOCP notifies worker threads when an I/O operation is complete.

- **Non-Blocking Operations:**  
  Functions like `WSARecv()` and `WSASend()` are used with OVERLAPPED structures. This ensures that no thread is blocked waiting for data, allowing high throughput and responsiveness.

### Worker Thread Pool

- **Thread Pool:**  
  A pool of worker threads is created (configurable by `NUM_THREADS`). Each thread waits on `GetQueuedCompletionStatus()` to handle I/O completions.
  
- **Efficient Concurrency:**  
  This design allows your server to efficiently manage thousands of simultaneous connections without spawning a thread per connection.

### Connection State and Keep-Alive

- **Persistent Connection Management:**  
  The library supports both "Connection: close" and keep-alive connections.  
  - For keep-alive, the per-connection state (buffers, OVERLAPPED structures, etc.) is maintained and reused.  
  - If a client specifies `"Connection: close"` (or omits keep-alive), the socket is closed after sending the response.
  
- **Buffering and Request Parsing:**  
  Incoming data is buffered per connection. Once a complete request (based on headers and Content-Length) is received, it is parsed once using the `parseHttpRequest()` function, then processed through middleware and routing.

## API Reference

### HttpServer Class

The `HttpServer` class is the core of the library. It manages server initialization, routing, middleware, and asynchronous I/O using IOCP.

- **`int initServer(int addressFamily, int type, int protocol, int port)`**  
  Initializes the server socket, binds to the specified port, starts listening, creates the IOCP handle, and launches worker threads.

- **`void serverListen(const SOCKET &serverSocket)`**  
  Accepts new connections, associates them with the IOCP handle, and issues asynchronous receive operations.

- **`void use(std::function<void(Req&, Res&, long long&)> middleware)`**  
  Adds a global middleware function executed for every incoming request.

- **Routing Methods:**  
  - **`void Get(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler)`**  
  - **`void Post(...)`**  
  - **`void Put(...)`**  
  - **`void Patch(...)`**  
  - **`void Delete(...)`**  
  Registers routes with associated middleware and handlers.

### Request (Req) Class

Represents an incoming HTTP request, containing:

- **`std::string method`**: HTTP method (GET, POST, etc.)
- **`std::string path`**: URL path
- **`std::string protocol`**: HTTP protocol version (default "HTTP/1.1")
- **`std::string payload`**: Request body data
- **`std::unordered_map<std::string, std::string> headers`**: HTTP headers
- **`std::unordered_map<std::string, std::string> queryParameters`**: Parsed URL query parameters
- **`JSONValue body`**: Parsed JSON body (populated by middleware)

### Response (Res) Class

Used to build and send an HTTP response:

- **`Res* status(int statusCode)`**: Sets the HTTP status code.
- **`Res* json(const JSONValue &j)`**: Sets a JSON response with appropriate headers.
- **`Res* send(const std::string data)`**: Sets a plain-text response.
- **`Res* setHeader(const std::string key, const std::string value)`**: Sets a custom header.

### JSON Handling

- **`JSONValue` Class:**  
  Represents JSON data and supports types: null, bool, double, string, array, and object.
  - **`std::string stringify() const`**: Converts the JSONValue into a JSON-formatted string.

- **`JSONParser` Class:**  
  Parses a JSON-formatted string into a `JSONValue`. It includes helper functions for parsing strings, numbers, objects, arrays, booleans, and null.

### Utilities

The library provides helper functions to process strings:

- **`std::string trim(const std::string str)`**  
  Removes leading and trailing whitespace.  
  (Defined in `utils.cpp` with its declaration in `utils.h`.)

- **`std::vector<std::string> split(const std::string str, const char delim)`**  
  Splits a string by the specified delimiter.

## License

This library is provided "as-is" without any express or implied warranty. You are free to use and modify it as needed.
