# Boltpp – A High-Performance C++ HTTP Server 🚀

Boltpp is a lightweight, high-performance, and expressive C++17 HTTP server library inspired by the simplicity of Express.js. Built from the ground up using Winsock2 and modern C++ features, Boltpp provides a powerful toolkit for creating fast and scalable web services with zero external dependencies. It features a robust routing system, flexible middleware support, a built-in JSON parser, and asynchronous, multithreaded request handling right out of the box. ⚡️

---

## 📚 Table of Contents

- [Boltpp – A High-Performance C++ HTTP Server 🚀](#boltpp--a-high-performance-c-http-server-)
  - [📚 Table of Contents](#-table-of-contents)
  - [✨ Key Features](#-key-features)
  - [💡 Why Choose Boltpp?](#-why-choose-boltpp)
  - [🧰 Installation and Setup](#-installation-and-setup)
    - [Prerequisites](#prerequisites)
    - [Option 1: As a Git Submodule (Recommended)](#option-1-as-a-git-submodule-recommended)
    - [Option 2: Manual Build and System-Wide Install](#option-2-manual-build-and-system-wide-install)
  - [🛫 Getting Started: Your First Server](#-getting-started-your-first-server)
  - [📘 API Reference](#-api-reference)
    - [The `HttpServer` Class](#the-httpserver-class)
    - [Routing](#routing)
    - [Middleware](#middleware)
    - [The `Request` Object](#the-request-object)
    - [The `Response` Object](#the-response-object)
  - [🔒 Configuring CORS](#-configuring-cors)
  - [📄 Working with JSON](#-working-with-json)
    - [Creating a `JSONValue`](#creating-a-jsonvalue)
    - [Modifying a `JSONValue`](#modifying-a-jsonvalue)
    - [Accessing Data (Type-Safe)](#accessing-data-type-safe)
    - [Stringifying a `JSONValue`](#stringifying-a-jsonvalue)
  - [📊 Feature Comparison](#-feature-comparison)
  - [🗺️ Project Roadmap](#️-project-roadmap)
  - [🤝 Contributing](#-contributing)
  - [📧 Contact](#-contact)

---

## ✨ Key Features

- 🔹 **Express.js-Inspired API**: A simple and intuitive interface that feels familiar to Node.js developers.
- 🔹 **High-Performance I/O**: Asynchronous request handling using Windows IOCP for maximum throughput.
- 🔹 **Powerful Routing**: Full support for all standard HTTP methods, parameterized paths (`/users/:id`), and query strings.
- 🔹 **Flexible Middleware**: Chain global or route-specific middleware to handle logging, authentication, parsing, and more.
- 🔹 **Built-in Parsers**: Out-of-the-box support for `application/json` and `application/x-www-form-urlencoded` request bodies.
- 🔹 **Multithreading**: A configurable worker thread pool to process incoming requests concurrently.
- 🔹 **Zero Dependencies**: Built with pure C++17 and the native Winsock2 library. No external libraries needed.
- 🔹 **Connection Management**: Supports HTTP Keep-Alive for persistent connections, reducing latency.

---

## 💡 Why Choose Boltpp?

-   **Simplicity and Productivity**: Get a server up and running in minutes with an API designed for clarity and ease of use. Focus on your application logic, not boilerplate.
-   **Native Performance**: Leverage the raw power of C++ for applications where every microsecond counts, such as game backends, real-time APIs, and high-traffic web services.
-   **Total Control**: With zero dependencies, Boltpp offers a lightweight, transparent, and easily integrated solution for any C++ project.

---

## 🧰 Installation and Setup

### Prerequisites

-   A C++17 compliant compiler (e.g., MSVC, MinGW-w64)
-   CMake (version 3.10 or later)
-   Ninja or another build system (optional, but recommended)
-   Windows Operating System (due to Winsock2/IOCP)

### Option 1: As a Git Submodule (Recommended)

This is the modern, recommended approach for integrating libraries into your CMake project. It avoids system-wide installs and keeps dependencies self-contained.

1.  **Add Boltpp as a submodule to your project:**
    ```bash
    git submodule add https://github.com/your-username/Boltpp.git vendor/Boltpp
    ```

2.  **Configure your project's `CMakeLists.txt`:**
    ```cmake
    cmake_minimum_required(VERSION 3.10)
    project(MyAwesomeApp)

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Add the Boltpp subdirectory to your build
    add_subdirectory(vendor/Boltpp)

    # Add your executable
    add_executable(MyAwesomeApp main.cpp)

    # Link against the Boltpp library
    target_link_libraries(MyAwesomeApp PRIVATE Boltpp::Boltpp)
    ```

### Option 2: Manual Build and System-Wide Install

Follow these steps if you prefer to build and install the library on your system.

```bash
# 1. Clone the repository
git clone https://github.com/your-username/Boltpp.git
cd Boltpp

# 2. Create a build directory
mkdir build && cd build

# 3. Configure with CMake (specify your install location)
cmake .. -G "Ninja" -DCMAKE_INSTALL_PREFIX="C:/your/install/path/Boltpp"

# 4. Build the library
ninja

# 5. Install the library
ninja install
```
Then, in your project's `CMakeLists.txt`, use `find_package` to locate and link the installed library.

---

## 🛫 Getting Started: Your First Server

The following example demonstrates a simple server with global middleware, a GET route that returns JSON, and a POST route.

```cpp
#include <iostream>

#include "httpserver.h"
#include "json.h"

int main() {
    // 1. Create an instance of the HttpServer
    HttpServer server;

    // 2. Configure Cross-Origin Resource Sharing (CORS)
    server.createCorsConfig([](CorsConfig &config) {
        config.allowedOrigins = {"*"}; // Allow requests from any origin
        config.allowedMethods = {"POST", "PUT", "GET", "DELETE", "OPTIONS"};
        config.withCredentials = false;
    });

    // 3. Register global middleware. These run for every request in order.
    server.use([](Request &request, Response &response, long long &next) {
        std::cout << "Request received for URL: " << request.url << std::endl;
        next++; // Pass control to the next middleware
    });
    server.use([](Request &request, Response &response, long long &next) {
        if (!request.payload.empty()) {
            std::cout << "Request payload: " << request.payload << std::endl;
        }
        next++;
    });

    // 4. Define a GET route that returns JSON data
    server.Get("/user", [](Request &request, Response &response) {
        JSONValue::Object userInfo;
        userInfo["name"] = "Alex";
        userInfo["details"] = JSONValue::Object{
            {"age", 30.0},
            {"height", 160.0},
        };
        response.status(200).json(userInfo);
    });

    // 5. Define a root GET route
    server.Get("/", [](Request &request, Response &response) {
        response.status(200).send("Hello World!");
    });

    // 6. Initialize and start the server on port 9000
    // This function is blocking and will run until the program is terminated.
    try {
        server.initServer(9000, []() {
            std::cout << "Server is running on port 9000..." << std::endl;
        });
    } catch (const std::runtime_error& e) {
        std::cerr << "Error initializing server: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

---

## 📘 API Reference

### The `HttpServer` Class

This is the main class for configuring and running your web server.

-   `use(middleware)`
    Adds a global middleware that will be executed for every incoming request, in the order it was added.
    -   **`middleware`**: A function with the signature `void(Request&, Response&, long long& next)`.

-   `setThreads(unsigned int count)`
    Sets the number of worker threads in the thread pool. Defaults to 1.
    -   **`count`**: The number of threads to handle requests concurrently. A good starting point is `std::thread::hardware_concurrency()`.

-   `initServer(int port, callback, ...)`
    Initializes all server components (sockets, IOCP, threads) and binds to the specified port. This is a **blocking call** that starts the server's main event loop.
    -   **`port`**: The network port to listen on (e.g., `8080`).
    -   **`callback`**: An optional `std::function<void()>` that is executed once the server is successfully initialized and ready to accept connections.

-   `Get(path, middlewares, handler)` / `Post(...)` / `Put(...)` / `Patch(...)` / `Delete(...)`
    Registers a handler for a specific HTTP method and path. See the [Routing](#routing) section for details.

### Routing

Boltpp supports static, parameterized, and query-based routes.

-   **Static Routes**: Match an exact path.
    ```cpp
    server.Get("/about", ...);
    ```

-   **Parameterized Routes**: Use the `:param` syntax to capture dynamic segments of a URL.
    ```cpp
    // Matches /users/123, /users/abc, etc.
    server.Get("/users/:id", [](Request& req, Response& res) {
        // Access the captured parameter
        std::string userId = req.pathParameters["id"];
        res.send("Fetching user with ID: " + userId);
    });
    ```

-   **Query Parameters**: Automatically parsed from the URL's query string.
    ```cpp
    // For a request to /search?q=boltpp&lang=cpp
    server.Get("/search", [](Request& req, Response& res) {
        std::string query = req.queryParameters["q"];      // "boltpp"
        std::string lang = req.queryParameters["lang"];    // "cpp"
        res.send("Searching for " + query + " in " + lang);
    });
    ```

### Middleware

Middleware are functions that execute in a sequence before the final route handler. They are ideal for cross-cutting concerns.

-   **Signature**: `void(Request& req, Response& res, long long& next)`
-   **Control Flow**:
    -   `next++`: Call this to pass control to the next middleware in the chain. If you don't call it, the request handling stops.
    -   `next = -1`: **Halt execution immediately.** No further middleware or the route handler will be called. You *must* send a response in the middleware if you do this.
    -   `next += 2`: (Advanced) Skips the next middleware in the chain. Use with caution.

-   **Example: Custom Logger Middleware**
    ```cpp
    void logger(Request& req, Response& res, long long& next) {
        std::cout << "Request received: " << req.method << " " << req.url << std::endl;
        next++; // Pass control to the next function
    }

    server.use(logger);
    ```

### The `Request` Object

Contains all information about an incoming HTTP request.

| Property          | Type                                                 | Description                                            |
| ----------------- | ---------------------------------------------------- | ------------------------------------------------------ |
| `method`          | `std::string`                                        | The HTTP method (e.g., "GET", "POST").                 |
| `url`             | `std::string`                                        | The full original URL, including path and query string. |
| `path`            | `std::string`                                        | The path portion of the URL, without the query string. |
| `headers`         | `std::unordered_map<std::string, std::string>`       | A map of all HTTP request headers.                     |
| `pathParameters`  | `std::unordered_map<std::string, std::string>`       | Key-value pairs of captured route parameters.          |
| `queryParameters` | `std::unordered_map<std::string, std::string>`       | Key-value pairs from the URL query string.             |
| `payload`         | `std::string`                                        | The raw, unparsed request body.                        |
| `body`            | `JSONValue`                                          | The parsed request body (if a parser middleware ran).  |

### The `Response` Object

Used to construct and send the HTTP response back to the client. Methods are chainable.

| Method                    | Return Type | Description                                                              |
| ------------------------- | ----------- | ------------------------------------------------------------------------ |
| `status(int code)`        | `Response&` | Sets the HTTP status code (e.g., `200`, `404`).                            |
| `send(std::string_view)`  | `Response&` | Sets the response body as plain text and sets `Content-Type: text/plain`. |
| `json(const JSONValue&)`  | `Response&` | Stringifies the `JSONValue` and sets `Content-Type: application/json`.     |
| `setHeader(key, value)`   | `Response&` | Sets a custom HTTP response header.                                      |

**Example of Chaining:**
```cpp
res.status(404)
   .setHeader("X-Custom-Header", "Error")
   .send("Not Found");
```
---

## 🔒 Configuring CORS

Boltpp provides built-in support for CORS, which is essential for allowing web applications from different domains to securely access your API.

### `server.createCorsConfig(configurer)`

This method accepts a lambda function that gives you mutable access to a `CorsConfig` object, allowing you to define your server's CORS policy.

```cpp
server.createCorsConfig([](CorsConfig& config) {
    // Allow requests from any origin
    config.allowedOrigins = {"*"};

    // Or, for better security, specify allowed origins explicitly
    // config.allowedOrigins = {"http://localhost:3000", "https://my-frontend.com"};

    // Specify the HTTP methods clients are allowed to use
    config.allowedMethods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};

    // Specify the headers clients can include in their requests
    config.allowedHeaders = {"Content-Type", "Authorization"};

    // Set to true to allow cookies and credentials to be sent with requests
    config.withCredentials = false;
});
```

### `CorsConfig` Fields

-   🔹 **`allowedOrigins`**: An `std::unordered_set<std::string>` of origins permitted to make requests. Use `"*"` to allow any origin.
-   🔹 **`allowedMethods`**: An `std::unordered_set<std::string>` of allowed HTTP methods (e.g., `"GET"`, `"POST"`).
-   🔹 **`allowedHeaders`**: An `std::unordered_set<std::string>` of HTTP headers that the client may send.
-   🔹 **`exposedHeaders`**: An `std::unordered_set<std::string>` of response headers that the browser is allowed to access.
-   🔹 **`withCredentials`**: A `bool`. If `true`, it allows browsers to send credentials (like cookies).

**Important Note**: For security reasons, if `withCredentials` is set to `true`, you **cannot** use `"*"` for `allowedOrigins`. You must specify the exact origin(s).

---

## 📄 Working with JSON

Boltpp includes a powerful, type-safe `JSONValue` class for handling JSON data.

### Creating a `JSONValue`

Use initializers for clear and concise construction.

```cpp
JSONValue::Object user; // Creates a JSON object
user["name"] = "Alice";
user["age"] = 30.0;
user["isStudent"] = false;
user["courses"] = JSONValue::Array{"History", "Math"};
```

### Modifying a `JSONValue`

A default-constructed `JSONValue` is `null`. To use it as an object or array, you must first assign it the correct type.

```cpp
// Method 1: Initialize with the desired type
JSONValue data(JSONValue::Object);
data["key"] = "value"; // Correct

// Method 2: Assign an object or array to it
JSONValue items;
items = JSONValue::Array{}; // Now it's an array
items = "First Item";     // This is now an error, as array is empty, and operator[] doesn't emplace
```
*NOTE:* `operator[]` for arrays in `JSONValue` does not `emplace_back`, it only provides access.

### Accessing Data (Type-Safe)

Use the `.asType()` methods to retrieve values. Accessing a value with the wrong type will throw a `json_type_error` exception.

```cpp
try {
    std::string name = user["name"].asString();
    double age = user["age"].asDouble();
    bool isStudent = user["isStudent"].asBool();
} catch (const json_type_error& e) {
    std::cerr << "JSON type error: " << e.what() << std::endl;
}
```

### Stringifying a `JSONValue`

Convert a `JSONValue` object into a standard JSON string.

```cpp
std::string jsonString = user.stringify();
// {"name":"Alice","age":30.0,"isStudent":false,"courses":["History","Math"]}
```

---

## 🗺️ Project Roadmap

Boltpp is actively developing. Future enhancements include:

-   ✅ **Cross-Platform Support**: Implement `epoll` (Linux) and `kqueue` (macOS) for the I/O backend.
-   ✅ **HTTPS/TLS Support**: Integrated TLS for secure communication.
-   ✅ **WebSocket Support**: Enable real-time, bidirectional communication.
-   ✅ **HTTP/2 Support**: Improve performance with the latest HTTP standard.
-   ✅ **Enhanced Error Handling**: More granular error types and better diagnostics.

---

## 🤝 Contributing

Contributions are welcome! If you have suggestions, find a bug, or want to add a feature, please feel free to open an issue or submit a pull request.

---

## 📧 Contact

For questions or collaboration, please reach out:

-   **Email**: [sgobind577@gmail.com]
-   **LinkedIn**: [https://www.linkedin.com/in/gobind-singh-maan-2548a5157/]
