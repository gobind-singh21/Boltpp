#pragma once

#include <unordered_map>
#include <string>

#include "json.h"

class Req {
public:
  Req() : method(""), path(""), protocol(""), payload(""), queryParameters({}), headers({}) {}
  Req(std::string _method, std::string _path, std::string _protocol, std::string _payload,
      std::unordered_map<std::string, std::string> query,
      std::unordered_map<std::string, std::string> _headers)
      : method(_method), path(_path), protocol(_protocol), payload(_payload),
        queryParameters(query), headers(_headers) {}
  Req(const Req &req)
      : method(req.method), path(req.path), protocol(req.protocol), payload(req.payload),
        queryParameters(req.queryParameters), headers(req.headers) {}

  std::string method;
  std::string path;
  std::string protocol = "HTTP/1.1";
  std::string payload;
  std::unordered_map<std::string, std::string> queryParameters;
  std::unordered_map<std::string, std::string> headers;
  JSONValue body;
};