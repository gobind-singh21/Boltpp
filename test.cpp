#include "httpserver.h"

int main() {
  HttpServer server;
  
  server.Get("/", {}, [](Req &req, Res &res) {
    res.send("Hello from root path")->status(200);
  });

  server.Get("/user", {}, [](Req &req, Res &res) {
    res.send(req.path);
  });

  server.Post("/new-user", {}, [](Req &req, Res &res) {
    std::unordered_map<std::string, std::string> user;
    user["name"] = "Json";
    user["age"] = "34";
    res.json(user);
  });
  
  server.serverInit(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  getchar();
  
  return 0;
}