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

    if(const JSONValue::Object *obj = std::get_if<JSONValue::Object>(&(userJson.value))) {
      auto it = obj->find("name");
      if(it != obj->end()) {
        if(const std::string *name = std::get_if<std::string>(&it->second.value)) {
          std::cout << "name: " << *name << std::endl;
        }
      }
    }
    res.json(userJson)->status(201);
  });

  SOCKET serverSocket = server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  server.serverListen(serverSocket);

  getchar();
  return 0;
}