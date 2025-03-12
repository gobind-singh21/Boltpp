#include <functional>
#include <string>
#include <vector>

#include "request.h"
#include "response.h"

void cors(std::vector<std::string> methods, std::string host, Req &req, Res &res, int &next) {
  if(req.headers["Host"] != host) {
    next = -1;
    return;
  }
  for(const std::string &method : methods) {
    if(method.compare(req.method) == 0) {
      next++;
      return;
    }
  }
  next = -1;
}

void contentParser(Req &req, Res &res, int &next) {
  const std::string contentType = req.headers["Content-Type"];
  if(contentType.find("application/json") != std::string::npos) {
    std::string boundary = "";
    size_t size = req.payload.length(), i = 0, lastIndex = size - 1;
    while(i < lastIndex && req.payload[i] == '\r' && req.payload[i + 1] == '\n') {
      boundary += req.payload[i];
    }
  } else if(contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
    size_t size = req.payload.length();
    for(size_t i = 0; i < size; i++) {

    }
  } else if(contentType.find("text/plain") != std::string::npos) {

  }
  next++;
}

void fileParser(Req &req, Res &res, int &next) {
  const std::string contentType = req.headers["Content-Type"];
  if(contentType.find("multipart/form-data") != std::string::npos) {

  }
  next++;
}