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

  inline void setProtocol(const std::string protocol) { this->protocol = protocol; }
  inline std::string getProtocol() const { return protocol; }
  inline std::string getPayload() const { return payload; }
  inline int getStatusCode() const { return statusCode; }

  Res* status(int statusCode) {
    this->statusCode = statusCode;
    return this;
  }

  Res* json(const JSONValue &j) {
    std::string jsonString = j.stringify();
    this->payload = jsonString;
    headers["Content-Type"] = "application/json";
    headers["Content-Length"] = std::to_string(jsonString.length());
    return this;
  }

  Res* send(std::string data) {
    this->payload = data;
    headers["Content-Type"] = "text/plain; charset=UTF-8";
    headers["Content-Length"] = std::to_string(data.length());
    return this;
  }

  Res* setHeader(const std::string key, const std::string value) {
    headers[key] = value;
    return this;
  }
};