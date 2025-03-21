# C++ HTTP Server Library

A lightweight HTTP server library written in C++ designed for Windows. This library provides a simple yet flexible framework for building web servers with support for routing, middleware, and JSON parsing. It leverages Winsock2 for network communication and uses modern C++ features (such as std::variant) for JSON handling.

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
  - [API Reference](#api-reference)
    - [HttpServer Class](#httpserver-class)
    - [Request (Req) Class](#request-req-class)
    - [Response (Res) Class](#response-res-class)
    - [JSON Handling](#json-handling)
    - [Utilities](#utilities)

## Features

- **Basic HTTP Server**: Create a server that listens on a specified port.
- **Routing**: Define routes for HTTP methods such as GET, POST, PUT, PATCH, and DELETE.
- **Middleware Support**: Chain middleware functions to process requests (e.g., for parsing JSON bodies).
- **JSON Parsing and Stringifying**: Convert between JSON strings and in-memory objects.
- **Query Parameter Parsing**: Automatically extract query parameters from request URLs.
- **Threaded Request Handling**: Each client connection is handled in a separate thread.

## Requirements

- **Operating System**: Windows (using Winsock2 for networking)
- **Compiler**: C++17 compliant compiler
- **Libraries**: Standard C++ libraries; Winsock2 (ws2_32.lib)

## Installation and Compilation

1. **Clone or Download the Repository** containing the following key files:
   - `httpserver.h` and `httpserver.cpp`
   - `json.h` and `json.cpp`
   - `request.h`
   - `response.h` and `response.cpp`
   - `middlewares.h`
   - `utils.h`

2. **Compile the Library**  
   When using Visual Studio or another Windows C++ compiler, ensure that you link against `ws2_32.lib`. For example, if using the command line with `cl`:

   ```bash
   cl /EHsc httpserver.cpp json.cpp response.cpp main.cpp /link ws2_32.lib
   ```

   Replace `main.cpp` with your application source file.

## Usage

### Initializing the Server

Create an instance of the `HttpServer` class and initialize it by specifying the address family, socket type, protocol, and port number. For example:

```cpp
#include "httpserver.h"

int main() {
  HttpServer server;
  // Initialize server for IPv4, TCP, and port 8080
  int serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8080);
  
  // The server starts listening on a separate thread
  // Your application can continue running or perform other tasks

  while(true) {
    // Main thread work or sleep
  }
  
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
      res.send("Hello, world!");
    }
  );

  while(true) { }
  return 0;
}
```

Similarly, you can use `Post()`, `Put()`, `Patch()`, and `Delete()` methods to define routes for other HTTP methods.

### Middleware Support

Middleware functions allow you to preprocess requests before reaching the route handler. The library supports global middleware (applied to every request) using the `use()` method, and route-specific middleware as part of the route registration.

Example using a JSON body parser middleware:

```cpp
#include "httpserver.h"
#include "middlewares.h"

int main() {
  HttpServer server;
  server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8080);

  // Apply global middleware to parse JSON bodies
  server.use(JsonBodyParser);

  // Define a POST route that expects JSON input
  server.Post("/data", {},
    [](Req &req, Res &res) {
      // After middleware, req.body holds the parsed JSON (of type JSONValue)
      std::string responseText = "Received JSON: " + req.body.stringify();
      res.send(responseText);
    }
  );

  while(true) { }
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
  // Create a JSON object
  JSONValue::Object obj;
  obj["message"] = JSONValue("Hello, JSON!");
  JSONValue jsonResponse(obj);
  
  // Send JSON response
  res.json(jsonResponse)->status(200);
}
```

## API Reference

### HttpServer Class

The `HttpServer` class is the core of the library and manages server initialization, routing, and request handling.

- **`int initServer(int addressFamily, int type, int protocol, int port)`**  
  Initializes the server, binds to the specified port, and starts listening for client connections.

- **`void use(std::function<void(Req&, Res&, long long&)> middleware)`**  
  Adds a global middleware function that is executed for every incoming request.

- **Routing Methods:**  
  - **`void Get(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler)`**
  - **`void Post(...)`**
  - **`void Put(...)`**
  - **`void Patch(...)`**
  - **`void Delete(...)`**  
  Registers a route with its associated middleware and handler.

### Request (Req) Class

The `Req` class represents an incoming HTTP request. It contains:

- **`std::string method`**: HTTP method (e.g., GET, POST)
- **`std::string path`**: Requested URL path
- **`std::string protocol`**: HTTP protocol version (default "HTTP/1.1")
- **`std::string payload`**: Request body data
- **`std::unordered_map<std::string, std::string> queryParameters`**: Parsed query parameters from the URL
- **`std::unordered_map<std::string, std::string> headers`**: HTTP headers
- **`JSONValue body`**: Parsed JSON body (populated by JSON middleware)

### Response (Res) Class

The `Res` class is used to build and send an HTTP response. It offers the following methods:

- **`Res* status(int statusCode)`**: Sets the HTTP status code.
- **`Res* json(const JSONValue &j)`**: Serializes a JSONValue and sets the appropriate headers for a JSON response.
- **`Res* send(const std::string data)`**: Sets the response payload for plain text responses.
- **`Res* setHeader(const std::string key, const std::string value)`**: Sets a custom header in the response.

### JSON Handling

- **`JSONValue` Class**  
  Represents a JSON value and supports types: null, bool, double, string, array, and object.  
  - **`std::string stringify() const`**: Converts the JSONValue to a JSON-formatted string.

- **`JSONParser` Class**  
  Parses a JSON-formatted string into a `JSONValue`. Key parsing methods include:
  - **`parseString()`**
  - **`parseNumber()`**
  - **`parseObject()`**
  - **`parseArray()`**
  - **`parseBoolean()`**
  - **`parseNull()`**  
  Use the `parse()` method to parse an entire JSON string.

### Utilities

The library provides helper functions defined in `utils.h`:

- **`std::string trim(const std::string str)`**: Removes leading and trailing whitespace.
- **`std::vector<std::string> split(const std::string str, const char delim)`**: Splits a string by a specified delimiter.
