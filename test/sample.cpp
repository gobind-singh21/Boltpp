#include <iostream>

#include "httpserver.h"

int main() {
  HttpServer server;

  server.createCorsConfig([](CorsConfig &config) {
    config.allowedOrigins = {"*"};
    config.allowedMethods = {"POST", "PUT", "GET", "DELETE", "OPTIONS"};
    config.withCredentials = false;
  });

  server.use([](Request &request, Response &response, long long &next) {
    std::cout << "Request received" << std::endl;
    next++;
  });
  server.use([](Request &request, Response &response, long long &next) {
    std::cout << request.payload << std::endl;
    next++;
  });

  server.Get("/user", [](Request &request, Response &response) {
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

    response.json(userJson).status(201);
  });

  server.initServer(9000, []() {
    std::cout << "Server listening on server port 9000" << std::endl;
  });

  return 0;
}