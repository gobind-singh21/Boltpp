# CPP HTTP Server library

- This library is made so that you can make Backend in c++ with coding style on NodeJS.

## PREREQUISITES

- OS: Windows (with winsock2.dll in it)
- Little bit knowledge of socket Programming not much
- C++ version: Atleast C++17 Atmax C++20 (C++23 Changes Implementation for JSON a bit so I'll look for it in the future)

## HOW TO USE

- ### STEP 1

  - Include the httpserver.h in your main c++ program.
    - ```#include "httpserver.h"```

- ### STEP 2

  - Make an object for HttpServer class in your c++ code.
    - ```HttpServer httpServer;```

- ### STEP 3

  - Define routes first (You can define them later but it is recommended to define it before initialization so that you don't encounter any unexpected behaviour (I'll soon look for its fix but it is what it for now)).
  - See test.cpp to get an idea for API end point example.

- ### STEP 4

  - Initialize server.
    - ```httpServer.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);```
      - AF_INET : This is IPv4 address family, denoting server to use IPv4 address.
      - SOCK_STREAM : This is for server to use TCP connection (Right now it only supports TCP).
      - IPPROTO_TCP: This is to specify to use TCP protocol (again it support only TCP as of now).
      - 9000: This is the Port number on which your backend will run.

### NOTE

- This library is currently in progress with major functionalities in the process of their implementation
- There are a lot of optimizations and security enhancements to be made as of now
- Right now it has been keeping in mind that you are using it on windows (Linux version will be made after windows implementation of it is completed).
- It only supports TCP (UDP support will be added in future).
- Support for Websocket will arrive later on.
- Supports only HTTP/1.1 (some features are to be implemented as of now).
- Doesn't have support of middleware as of now (even though I have kept middleware variables but they are not implemented).
- Support for JSON parsing in request and payload of http request is on work.
