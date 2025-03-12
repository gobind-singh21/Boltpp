#pragma once

#include <unordered_map>
#include <string>

#include "json.h"

class Res {
  int statusCode = 200;
  std::string payload;
  std::string protocol;

public:
  std::unordered_map<std::string, std::string> headers;

  inline void setProtocol(const std::string protocol) { this->protocol = protocol;}
  inline std::string getProtocol() const { return protocol; }
  inline std::string getPayload() const { return payload; }
  inline int getStatusCode() const { return statusCode; }

  Res* status(int statusCode);

  Res* json(const JSONValue &j);

  Res* send(const std::string data);

  Res* setHeader(const std::string key, const std::string value);
};