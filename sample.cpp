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
    res.json(userJson)->status(201);
  });

  server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  getchar();
  return 0;
}