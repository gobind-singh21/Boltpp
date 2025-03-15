#include "request.h"
#include "response.h"

class BodyParser {
public:
  static inline auto json = [](Req &req, Res &res, long long &next) {
    if(req.headers["Content-Type"].find("application/json") != std::string::npos) {
      try {
        req.body = JSONParser(req.payload).parse();
      } catch (const std::exception &e) {
        res.send("Bad Request")->status(400);
        next = -1;
        return;
      }
    }
    next++;
  };
  static inline auto urlEncoded = [](Req &req, Res &res, long long &next) {
    if(req.headers["Content-Type"].find("application/x-www-form-urlencoded") != std::string::npos) {
      // implementation remaining
    }
    next++;
  };
};