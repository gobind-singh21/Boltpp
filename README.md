# Boltpp – High-Performance C++ HTTP Server Library 🚀🚀🚀

Boltpp is a high-performance, expressive, and lightweight C++ HTTP server library that mimics the simplicity of Express.js while leveraging the power and performance of native C++. Built from scratch using Winsock2 and standard C++ libraries, Boltpp enables fast HTTP request handling with built-in support for routing, middleware, JSON parsing, and multithreading. ⚡️⚙️🧵

---

## 📚 Table of Contents 🧭🗂️📝

- [Boltpp – High-Performance C++ HTTP Server Library 🚀🚀🚀](#boltpp--high-performance-c-http-server-library-)
  - [📚 Table of Contents 🧭🗂️📝](#-table-of-contents-️)
  - [🚀 Features 💡💥📌](#-features-)
  - [📂 Directory Structure 🗂️📁🔧](#-directory-structure-️)
  - [💪 Installation 🧰📦⚙️](#-installation-️)
    - [Step 1: Build and Install](#step-1-build-and-install)
    - [Step 2: Use in Your Project](#step-2-use-in-your-project)
  - [🏗️ Getting Started 🧪🗒️🛫](#️-getting-started-️)
    - [Example](#example)
  - [📜 API Documentation 📘📚🧾](#-api-documentation-)
    - [HttpServer Methods](#httpserver-methods)
    - [Middleware](#middleware)
    - [Request class 📨📥🔍](#request-class-)
    - [Response class 📤🧞✅](#response-class-)
    - [Creating API End points](#creating-api-end-points)
  - [🪧 JSONValue Utilities 🧠📄🔎](#-jsonvalue-utilities-)
    - [Construct JSON](#construct-json)
    - [Access JSON Fields](#access-json-fields)
    - [Stringify JSON](#stringify-json)
  - [🌟 Advantages over Other Frameworks 🏋️🧰💯](#-advantages-over-other-frameworks-️)
  - [🚫 Limitations (To Be Improved) 🗱️⚠️🔧](#-limitations-to-be-improved-️️)
  - [📧 Contact 💬👤📨](#-contact-)

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

  // Middleware that will run globally in the order in which they are mentioned in the code
  server.use(JsonBodyParser);
  server.use(UrlencodedBodyParser);

  // GET API End point on the path /user
  server.Get("/user", {}, [](Req &req, Res &res) {
    JSONValue::Object user{{"name", "John"}, {"details", JSONValue::Object{
      {"age", 30.0},
      {"hobbies", JSONValue::Array{"coding", "gaming"}
    }}};
    res.json(JSONValue(user))->status(200);
  });

  // POST API end point on the path /
  server.Post("/", {}, [](Request &req, Response &res) {
    res.send("Hello world!")->status(200);
  });

  // Setting number of worker threads to 4 (default 1)
  server.setThreads(4);

  // Initializing server
  server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  // Making server listen
  server.serverListen();

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
void middleware(Request &req, Response &res, long long &next);
```

Call `next++` to pass control to the next middleware/handler.

Additionally if you want to you can do something like `next += 2` or `next += 3` upto if you want to skip some middleware in the chain as per your logic.

Example:

- `next += 2` will skip the middleware right next in the chain
- `next += 3` will skip the 2 middlewares after it in the chain

Set `next = -1` to halt the control flow of middleware (Beware to set some response otherwise empty response will be sent to client as after this route handler won't be executed).

### Request class 📨📥🔍

- 🔹 `payload`: Raw request body string
- 🔹 `body`: JSONValue parsed object (JSON or URL-encoded)
- 🔹 `headers`: `std::unordered_map<std::string, std::string>`
- 🔹 `method`: HTTP method (GET, POST, etc.)
- 🔹 `url`: Request URL
- 🔹 `path`: Path at which request arrived

### Response class 📤🧞✅

- 🔹 `send(string)`: Send plain text response
- 🔹 `json(JSONValue)`: Send JSON response
- 🔹 `status(int)`: Set HTTP status code
- 🔹 `header(key, val)`: Set a response header

### Creating API End points

In order to create an API end point you can use `.Get()`, `.Post()`, `.Patch()`, `.Put()`, `.Delete()` from the HttpServer instance you created (Refer to `test/sample.cpp`).

Function signature for each method will be the same. Here for example I'll demonstrate creating a `POST` method

**Function Signature** : `httpServer.Post(std::string path, std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler)`

Seems complicated but let's break it down in simple terms.

- `std::string path` : Is the path at which you want your API End point.
  - Ofcourse you can also have path parameters in it
  - Example : `"/user/:id"` defines a path parameter `id` and now a request coming at `/user/123` will automatically be parsed and `id` will have value `123` which you can access by `pathParameter` attribute in the request object
  - There is also support for query parameters which are again also accessible by `queryParameter` attribute in the request object.
- `std::vector<std::function<void(Request&, Response&, long long&)>> middlewares` : These will be your route sepcific middlewares which will be executed right before dispatching the request to the handler. Let's break down its syntax as well
  - `std::function<void(Request&, Response&, long long&)>` : This is the lamba function in which it will have arguments as a reference to request object(`Request&`), reference to response object(`Response&`) and reference to next(`long long&`) (Don't worry you won't have to create request or response objects or next variables on your own all of that is handled internally).
- `std::function<void(Request&, Response&)> handler` : This is the handler for your route, where you will add whatever processing you need to do for that request. Again this is a lambda function with arguments as Reference to Request object(`Request&`) and reference to response object(`Response&`) (Again you don't have to declare them it will all be handled internally).

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

- #### NOTE

  ```cpp
  JSONValue user;
  user["name"] = "Alex";
  ```

  - This is invalid because default constructor for a JSONValue object initializes it to be null, so accessing a field `user["name"]` in that null would throw an exception.

  - In order to do it you can follow 2 ways
    - 1st : **Initialize the JSONValue object with your desired type via constructor**

      ```cpp
      JSONValue json(JSONValue::Object);
      json["name"] = "Alex";
      ```

      ```cpp
      JSONValue json(JSONValue::Array{30, 10});
      json[1] = 20;
      ```

    - 2nd : **Assign the object a value of your desired type after declaration**

      ```cpp
      JSONValue json;
      json = JSONValue::Object{
        {"name", "Alex"},
        {"hobbies", JSONValue::Array{"coding", "gaming"}}
      };
      ```

  If you try to access a field in json object which doesn't corresponds to its type it will throw an error.

  Example:
  
  ```cpp
  // Invalid as json object of type array and accessing a field via string implies you are trying to access like an object
  JSONValue json(JSONValue::Array{});
  json["name"];
  ```

  ```cpp
  // Again invalid as in order to access a field in an object of type object, string keys should be used
  JSONValue json(JSONValue::Object{});
  json[1];
  ```

### Access JSON Fields

```cpp
std::string name = user["name"].asString();
double age = user["age"].asNumber();
```

**NOTE** : `user["name"]` would return a reference to a JSONValue object stored in that field, in order to access value inside that field, use the helper function `user["name"].asString()`.

**NOTE** : If `user["name"]` contains string value type json object, and if you use `user["name"].asNumber()`. This will throw an exception as `.asNumber()` expects the object with which it is called to contain a double value. So use the helper function access value for the matching type.

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

- 🔹 Email: [sgobind577@gmail.com]
- 🔹 LinkedIn: [https://www.linkedin.com/in/gobind-singh-maan-2548a5157/]
