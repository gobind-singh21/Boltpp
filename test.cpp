#include "httpserver.h"

int main() {

  HttpServer server;

  server.use([](Req &req, Res &res, size_t &next) {
    std::cout << "Middleware 2 executed" << std::endl;
    next++;
  });
  server.use([](Req &req, Res &res, size_t &next) {
    std::cout << "Middleware 1 executed" << std::endl;
    next++;
  });
  
  server.Get("/", {}, [](Req &req, Res &res) {
    res.send("Hello from root path")->status(200);
  });

  server.Get("/user", {}, [](Req &req, Res &res) {
    res.send(req.path);
  });

  server.Post("/new-user", {[](Req &req, Res &res, size_t &next) {
    std::cout << "Specific middleware 1 executed" << std::endl;
    next++;
  }, [](Req &req, Res &res, size_t &next) {
    std::cout << "Specific middleware 2 executed" << std::endl;
    next++;
  }}, [](Req &req, Res &res) {
    JSONValue::Object user;
    user["name"] = "John";
    user["details"] = JSONValue::Object{
      {"age", 34.0},
      {"hobbies", JSONValue::Array{"coding", "reading"}}
    };

    JSONValue userJson(user);

    res.json(userJson)->status(200);
  });
  
  server.initServer(AF_INET, SOCK_STREAM, IPPROTO_TCP, 9000);

  getchar();
  
  return 0;
}