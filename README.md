With this library you can make a backend in C++ with a syntax styles like Express.js. So with this you can leverage the performance of C++ while having the simplicity of Express.js

# Getting Started

To make your server using this library, follow through this entire document to learn how you can make API endpoints, have middlewares, and configure CORS in your application.

## Basics:

To cover the basics, we’ll make a simple server which would send a “hello world” text back to the browser when we make a request on our server. After that, we’ll cover how to access the path and query parameters in a request, and at last, we’ll learn about how JSON works here.

### Making first server instance:

To get started we need an instance or object of the `HttpServer` class; here the instance or object of this class will be our entire server handling every request.

Before anything, make sure to include `httpserver.h` in your program which loads all of the necessary code you’ll need to make your server.

```cpp
HttpServer server;
```

**NOTE:** For the rest of the documentation, we’ll refer to this `HttpServer` object as our server instance.

Now that we have a server instance, let’s get it up and running.

To start the server we have a `.initServer()` method for our server instance which starts the server; one mandatory argument that you have to pass is the port number on which your server will run.

```cpp
server.initServer(8000);
```

Now this will start your server at the port `8000` on `localhost`.

**NOTE:** After this function is executed the control flow will not go forward so you should always put this function in just before the return statement or at the end if you use it inside a void function.

Now our server is running, let’s make a GET route to send “Hello World” back to the client, and don’t worry about other HTTP methods as the process for other methods will be the same as making a GET route.

```cpp
server.Get("/", [](Request &request, Response &response) {
  response.send("Hello World!").status(200);
});
```

Let’s try to break down everything in this API Endpoint so you can have a better understanding of what is happening under the hood.

1.  **.Get(const std::string path, std::function<void(Request&, Response&)>):**
    This method allows us to make a GET API Endpoint for a route. If you want to make for other methods follow this table to know about them:

    | API Endpoint METHOD | Function to be used |
    | ------------------- | ------------------- |
    | POST                | `.Post()`           |
    | PUT                 | `.Put()`            |
    | PATCH               | `.Patch()`          |
    | DELETE              | `.Delete()`         |

    If you don’t know about `std::function` check it out [here](https://en.cppreference.com/w/cpp/utility/functional/function) as this will be useful when we further go ahead and make things more clear and also consider checking lambda functions in C++ if you don’t know about them otherwise feel free to continue with this documentation.

    **NOTE:** Signature for each method is the same as others.

    We’ll see more about Request and Response classes later in the documentation.

2.  **“/” (const std::string path):**
    This is the route on which our GET method would be available. You can change it according to your own needs for example: “/user”, “/profile”, “/home/posts”

3.  **Route handler (std::function<void(Request&, Response&)>):**
    The function you see beside the route, is the function which executes on the request, here we call it as a route handler. Inside this route handler you can put your own code.

4.  **Request &request:**
    This is a reference to a `Request` object which represents the request received from the client wrapped in a class so that you can access data inside the request easily.

    **NOTE:** Just because it is a reference doesn’t mean you have to define it somewhere, the library will manage it internally, you just have to use the reference in your route handler to access the data of the request.

5.  **Response &response:**
    This is a reference to a `Response` object which represents the response server will send for the request it receives back to the client. In this you can edit what you’ll send back to the client according to your needs.

    **NOTE:** Again you don’t have to define a response object anywhere, the library will do it under the hood.
    **NOTE:** Response will be sent after the entire route handler is executed.

You may also have noticed in the above snippet that we have a `.send()` associated with the response object; this send method sets the payload of the response object to the string provided inside of it and content type as plain text, along with it there is a `.status()` which sets the status code for the response which is provided in the arguments of this function.

There are more methods for the response which we’ll cover later on.

**NOTE:** You can use another `.send()` after one send, and the final data sent to the server will be the one provided in the last `.send()` call.

After all this your final program for a hello world server will be like this:

```cpp
#include "httpserver.h"

int main() {
    // Server instance creation
    HttpServer server;

    // Defining Route
    server.Get("/", [](Request &request, Response &response) {
        // Setting response payload and status code
        response.send("Hello World!")
                .status(200);
    });

    // Starting server and now it won't go to return statement as here the control flow halts
    server.initServer(8000);

    return 0;
}
```

## Getting Query and Path parameters

In a lot of scenarios you would like to send path and query parameters in your requests. So let’s see how we can access them.

### Query parameter:

Query parameters are values that are sent in key value pairs in the request url.
For example:

`http://example.com/user?id=123&name=alex`

In this case over here we have 2 query parameter key value pairs separated by ‘&’:

*   `id = 123`
*   `name = alex`

To access these key value pairs the request object has a member called `query_parameters` which is an `unordered_map` and stores key value pairs of strings.

```cpp
server.Get("/", [](Request &request, Response &response) {
    std::string id = request.query_parameters["id"];
    std::string name = request.query_parameters["name"];
    std::cout << id << ' ' << name << std::endl;
    response.send("Hello World!")
            .status(200);
});
```

### Path parameter:

Path parameters are directly included in the route path after a forward slash.

You can make a route with parameter by having the variable name after a `:`.

`"/user/:id"`

So in the above route id would its path parameter which can vary for different requests.
Example:

A request is made on `"/user/123"`

Then the value for `id` would be `123`

In order to access path parameter the request object has a member called `path_parameters` which is an `unordered_map` and stores key value pairs of strings.

```cpp
server.Get("/user/:id", [](Request &request, Response &response) {
    std::string id = request.path_parameters["id"];
    std::cout << "User id: " << id << std::endl;
    response.send("Hello user" + id).status(200);
});
```
