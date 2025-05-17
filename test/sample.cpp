#include <iostream>

#include "httpserver.h"

int main() {
  HttpServer server;

  server.use([](Request &request, Response &response, long long &next) {
    std::cout << "Request recieved" << std::endl;
    next++;
  });
  server.use([](Request &request, Response &response, long long &next) {
    std::cout << request.payload << std::endl;
    next++;
  });

  server.Get("/user", {}, [](Request &request, Response &response) {
    JSONValue::Object userInfo;
    userInfo["name"] = "Alex";
    userInfo["details"] = JSONValue({
      {"age", 30.0},
      {"height", 160.0},
    });
    JSONValue userJson(userInfo);

    userJson["name"] = "Gobind";

    std::cout << userJson["name"].asString() << std::endl;
    std::cout << userJson["details"]["age"].asDouble() << std::endl;

    response.json(userJson)->status(201);
  });

  SOCKET serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  server.serverListen();

  getchar();
  return 0;
}