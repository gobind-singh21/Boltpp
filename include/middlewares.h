#include "request.h"
#include "response.h"

#include <iostream>

inline auto JsonBodyParser = [](Req &req, Res &res, long long &next) {
  if(req.headers["Content-Type"].find("application/json") != std::string::npos) {
    try {
      req.body = JSONParser(req.payload).parse();
    } catch (const std::exception &e) {
      res.status(400)->send("Bad Request");
      next = -1;
      return;
    }
  }
  next++;
};

// under work
inline auto UrlencodedBodyParser = [](Req &req, Res &res, long long &next) {
  if(req.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {
    bool keyEnd = false;
    size_t length = req.payload.length();
    std::string key = "", value = "";
    key.reserve(length);
    value.reserve(length);
    for(size_t i = 0; i < length; i++) {

    }
  }
  next++;
};