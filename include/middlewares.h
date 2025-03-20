#include "request.h"
#include "response.h"

#include <iostream>

inline auto JsonBodyParser = [](Req &req, Res &res, long long &next) {
  std::cout << req.headers["Content-Type"] << std::endl;
  if(req.headers["Content-Type"].find("application/json") != std::string::npos) {
    std::cout << "Enter middleware control" << std::endl;
    try {
      JSONValue json = JSONParser(req.payload).parse();
      std::cout << json.stringify() << std::endl;
      req.body = json;
    } catch (const std::exception &e) {
      res.status(400)->send("Bad Request");
      next = -1;
      return;
    }
  }
  next++;
};

inline auto UrlencodedBodyParser = [](Req &req, Res &res, long long &next) {
  if(req.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {

  }
  next++;
};