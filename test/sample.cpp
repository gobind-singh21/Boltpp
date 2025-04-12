#include <iostream>

#include "httpserver.h"

int main() {
  HttpServer server;

  server.use([](Req &req, Res &res, long long &next) {
    std::cout << "Request recieved" << std::endl;
    next++;
  });
  server.use([](Req &req, Res &res, long long &next) {
    std::cout << req.payload << std::endl;
    next++;
  });

  server.Get("/user", {}, [](Req &req, Res &res) {
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

    res.json(userJson)->status(201);
  });

  SOCKET serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  server.serverListen(serverSocket);

  getchar();
  return 0;
}