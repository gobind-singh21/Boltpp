#pragma once

#include <winsock2.h>
#include <functional>
#include <vector>
#include <unordered_map>

#include "request.h"
#include "response.h"

#pragma comment(lib, "ws2_32.lib")

// using Middleware = std::function<void(Req&, Res&, std::function<void()>)>;

class HttpServer {
  SOCKET serverSocket;
  class Route {
  public:
    Route() : middlewares({}), handler([](Req &req, Res &res) {}) {}
    Route(std::vector<std::function<void(Req&, Res&, long long&)>> _middlewares,
          std::function<void(Req&, Res&)> _handler)
          : middlewares(_middlewares), handler(_handler) {}
    Route(const Route &route) : middlewares(route.middlewares), handler(route.handler) {}
  
    std::vector<std::function<void(Req&, Res&, long long&)>> middlewares;
    std::function<void(Req&, Res&)> handler;
  };

  std::unordered_map<SOCKET, std::string> socketBuffers;
  std::unordered_map<std::string, Route> allowed;
  std::vector<std::function<void(Req&, Res&, long long&)>> globalMiddlewares;

  static std::string getStatusCodeWord(const int statusCode);

  static std::string makeHttpResponse(const Res &res);

  static char urlEncodingCharacter(const std::string specialSequence);

  static void parseQueryParameters(Req &req);

  static void sendErrorResponse(Res &res, const SOCKET &clientSocket);
  
  static Req parseHttpRequest(const std::string &request, const SOCKET &clientSocket);

  void handleClientRequest(const SOCKET &clientSocket);

  void serverListen(const SOCKET &serverSocket);

public:
  HttpServer() : serverSocket(INVALID_SOCKET) {}
  HttpServer(const SOCKET s) : serverSocket(s) {}

  int initServer(int addressFamily, int type, int protocol, int port);

  inline void use(std::function<void(Req&, Res&, long long&)> middleware) {
    globalMiddlewares.emplace_back(middleware);
  }

  void Get(const std::string path,
           const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
           std::function<void(Req&, Res&)> handler);

  void Post(const std::string path,
            const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
            std::function<void(Req&, Res&)> handler);

  void Patch(const std::string path,
             const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
             std::function<void(Req&, Res&)> handler);

  void Put(const std::string path,
           const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
           std::function<void(Req&, Res&)> handler);

  void Delete(const std::string path,
              const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
              std::function<void(Req&, Res&)> handler);

  ~HttpServer() {
    closesocket(serverSocket);
    WSACleanup();
  }
};