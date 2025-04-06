# Boltpp – High-Performance C++ HTTP Server Library 🚀🚀🚀

Boltpp is a high-performance, expressive, and lightweight C++ HTTP server library that mimics the simplicity of Express.js while leveraging the power and performance of native C++. Built from scratch using Winsock2 and standard C++ libraries, Boltpp enables fast HTTP request handling with built-in support for routing, middleware, JSON parsing, and multithreading. ⚡️⚙️🧵

---

## 📚 Table of Contents 🧭🗂️📝

- [Features](features)
- [Directory Structure](directory-structure)
- [Installation](installation)
- [Getting Started](getting-started)
- [API Documentation](api-documentation)
- [JSONValue Utilities](jsonvalue-utilities)
- [Advantages over Other Frameworks](advantages-over-other-frameworks)
- [Limitations (To Be Improved)](limitations-to-be-improved)
- [Contact](contact)

---

## 🚀 Features 💡💥📌

- 🔹 **Simple and Expressive API** similar to Express.js
- 🔹 **Asynchronous I/O** via IOCP (Windows)
- 🔹 **Routing for all HTTP methods** (GET, POST, etc.)
- 🔹 **Middleware support** with next-based chaining
- 🔹 **Built-in JSON and URL-Encoded Parsers**
- 🔹 **Keep-alive and persistent connections**
- 🔹 **Multithreaded request handling** via a configurable thread pool
- 🔹 **No external dependencies** (pure C++ with Winsock2)

---

## 📂 Directory Structure 🗂️📁🔧

```dir
Boltpp/
├── include/
│   ├── httpserver.h
│   ├── request.h
│   ├── response.h
│   ├── json.h
│   ├── utils.h
│   └── middlewares.h
├── src/
│   ├── httpserver.cpp
│   ├── json.cpp
│   ├── response.cpp
│   └── utils.cpp
└── CMakeLists.txt
```

---

## 💪 Installation 🧰📦⚙️

### Step 1: Build and Install

```bash
🔹 git clone https://github.com/yourname/Boltpp.git
🔹 cd Boltpp
🔹 mkdir build && cd build
🔹 cmake .. -G "Ninja" -DCMAKE_INSTALL_PREFIX="C:/Program Files (x86)/Boltpp"
🔹 ninja
🔹 ninja install
```

> Requires: C++17, MinGW-w64 or MSVC, Winsock2 🎯🖥️🔗

### Step 2: Use in Your Project

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyApp)

set(CMAKE_CXX_STANDARD 17)
find_package(Boltpp REQUIRED)

add_executable(MyApp main.cpp)
target_link_libraries(MyApp Boltpp::Boltpp)
```

---

## 🏗️ Getting Started 🧪🗒️🛫

### Example

```cpp
#include "httpserver.h"
#include "request.h"
#include "middlewares.h"

int main() {
  HttpServer server;

  server.use(JsonBodyParser);
  server.use(UrlencodedBodyParser);

  server.Get("/user", {}, [](Req &req, Res &res) {
    JSONValue::Object user{{"name", "John"}, {"details", JSONValue::Object{
      {"age", 30.0},
      {"hobbies", JSONValue::Array{"coding", "gaming"}
    }}};
    res.json(JSONValue(user))->status(200);
  });

  server.Post("/", {}, [](Req &req, Res &res) {
    res.send("Hello world!")->status(200);
  });

  server.setThreads(4);

  SOCKET sock = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);
  server.serverListen(sock);

  getchar();
  return 0;
}
```

---

## 📜 API Documentation 📘📚🧾

### HttpServer Methods

- 🔹 **use(middleware)**: Add a middleware function to all routes
- 🔹 **Get/Post(path, middlewareList, handler)**: Register a route with optional middleware
- 🔹 **setThreads(int)**: Set number of worker threads
- 🔹 **initServer(domain, type, protocol, port)**: Initializes the server socket
- 🔹 **serverListen(socket)**: Starts accepting and handling requests

### Middleware

Middleware functions must match the following signature: 🪩🥷🔄

```cpp
void middleware(Req &req, Res &res, long long &next);
```

Call `next++` to pass control to the next middleware/handler.

### Req (Request) 📨📥🔍

- 🔹 `payload`: Raw request body string
- 🔹 `body`: JSONValue parsed object (JSON or URL-encoded)
- 🔹 `headers`: `std::unordered_map<std::string, std::string>`
- 🔹 `method`: HTTP method (GET, POST, etc.)
- 🔹 `url`: Request URL

### Res (Response) 📤🧞✅

- 🔹 `send(string)`: Send plain text response
- 🔹 `json(JSONValue)`: Send JSON response
- 🔹 `status(int)`: Set HTTP status code
- 🔹 `header(key, val)`: Set a response header

---

## 🪧 JSONValue Utilities 🧠📄🔎

The JSON parser is hand-built to support nested objects and arrays with ease. 🧬📚💡

### Construct JSON

```cpp
JSONValue user = JSONValue::Object{
  {"name", "Alice"},
  {"age", 25.0},
  {"tags", JSONValue::Array{"dev", "c++"}}
};
```

### Access JSON Fields

```cpp
std::string name = user["name"].asString();
double age = user["age"].asNumber();
```

### Stringify JSON

```cpp
std::string str = user.stringify();
```

---

## 🌟 Advantages over Other Frameworks 🏋️🧰💯

| Feature        | Boltpp   | Crow         | Drogon       |
| -------------- | -------- | ------------ | ------------ |
| Language       | C++17    | C++11        | C++14+       |
| Routing        | Yes      | Yes          | Yes          |
| Middleware     | Yes      | Yes          | Yes          |
| JSON Parsing   | Built-in | External     | Built-in     |
| Multithreading | Yes      | Yes          | Yes          |
| IO Model       | IOCP     | epoll/kqueue | epoll        |
| Simplicity     | ⭐⭐⭐⭐⭐    | ⭐⭐⭐          | ⭐⭐⭐⭐         |
| Dependencies   | None     | Boost (opt)  | libpqxx etc. |
| Compile Time   | Fast     | Medium       | Heavy        |

---

## 🚫 Limitations (To Be Improved) 🗱️⚠️🔧

- 🔹 No WebSocket or HTTP/2 support yet
- 🔹 Windows-specific IOCP implementation (POSIX epoll/kqueue planned)
- 🔹 Limited error handling APIs
- 🔹 No TLS support (planned)

---

## 📧 Contact 💬👤📨

For suggestions, issues, or collaboration:

- 🔹 Email: [[yourname@example.com](mailto\:yourname@example.com)]
- 🔹 LinkedIn: [linkedin.com/in/yourname]
