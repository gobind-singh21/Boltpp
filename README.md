# Boltpp - High Performance C++ HTTP Server Library

Boltpp is a lightweight and high-performance C++ HTTP server library designed for building HTTP servers with middleware support. Inspired by Express.js, it offers a simple and powerful API for defining routes, handling requests, and managing responses. It also includes a custom JSON parser and serializer for processing JSON data.

---

## Features

- Supports all HTTP methods: `GET`, `POST`, `PUT`, `PATCH`, `DELETE`
- Middleware support for request handling
- Custom JSON parsing and serialization
- Efficient request handling using `std::thread`
- URL query parameter parsing
- Request and response object handling

---

## Requirements

- C++17 or later
- CMake 3.10 or later
- Ninja (Recommended for building)

---

## Building the Library

### Cloning the Repository

Ensure your directory structure looks like this:

```file
Boltpp/
├── CMakeLists.txt          # Main CMake file
├── include/                # Public headers
├── src/                    # Private source files (DO NOT upload these to GitHub)
├── test/                   # Example or test applications
```

### Building with CMake and Ninja

```bash
mkdir build
cd build
cmake .. -G "Ninja"
ninja
```

### Installing the Library

```bash
ninja install
```

This will install the library to the default install location (e.g., `C:/Program Files (x86)/Boltpp/`). The installation includes:

- Public headers in the `include/` directory.
- Compiled binary files in the `lib/` directory.
- A `BoltppConfig.cmake` file in `lib/cmake/Boltpp/` for use with `find_package()`.

---

## Using the Library in a New Project

### Example `main.cpp`

```cpp
#include <iostream>
#include "httpserver.h"

int main() {
    HttpServer server;

    server.Get("/user", {}, [](Req &req, Res &res) {
        JSONValue::Object userInfo = { {"name", "Alex"}, {"age", 22.0} };
        res.json(JSONValue(userInfo))->status(200);
    });

    server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 8000);
    getchar();
    return 0;
}
```

### Example `CMakeLists.txt`

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boltpp REQUIRED)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp Boltpp::Boltpp)
```

### Building the Project

```bash
mkdir build
cd build
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/Boltpp"
ninja
```

---

## API Reference

### `HttpServer` Class

- **initServer(int, int, int, int)**: Initializes and starts the server.
- **use(std::function<void(Req&, Res&, long long&)>)**: Registers global middleware.
- **Get, Post, Put, Patch, Delete**: Define routes for various HTTP methods.

### `Req` Class

- **method**: The HTTP method (e.g., "GET", "POST").
- **path**: The URL path.
- **headers**: HTTP headers.
- **payload**: Request body.

### `Res` Class

- **status(int)**: Sets HTTP status code.
- **json(const JSONValue&)**: Sends a JSON response.
- **send(const std::string&)**: Sends a plain text response.
- **setHeader(const std::string&, const std::string&)**: Adds headers to the response.

---

## JSON Parser

### Parsing JSON Strings

```cpp
std::string jsonStr = R"({"name": "John", "age": 30, "isAdmin": true})";
JSONParser parser(jsonStr);
JSONValue jsonValue = parser.parse();
```

### Creating JSON Objects

```cpp
JSONValue::Object obj = {
    {"name", "Alice"},
    {"age", 25.0},
    {"hobbies", JSONValue::Array{"coding", "reading"}}
};
JSONValue json(obj);
std::cout << json.stringify() << std::endl;
```

---

### CMake Configuration

The `CMakeLists.txt` file is configured to:

- Build the library from the private `src/` files.
- Install only the public headers from the `include/` directory.
- Export a `BoltppConfig.cmake` file for use with `find_package()`.

### GitHub Repository Guidelines

- **Do not upload the `src/` directory.** Add it to `.gitignore` so only public headers, CMake files, and documentation are published.
- Provide precompiled binaries as GitHub Releases if needed.

---

## Contributing

If you would like to contribute, please fork the repository and submit pull requests. Include tests and update documentation as necessary.
