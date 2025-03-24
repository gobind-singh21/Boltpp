#include "response.h"

// inline void Res::setProtocol(const std::string protocol) {
//   this->protocol = protocol;
// }

Res* Res::status(int statusCode) {
  this->statusCode = statusCode;
  return this;
}

Res* Res::json(const JSONValue &j) {
  std::string jsonString = j.stringify();
  this->payload = jsonString;
  headers["Content-Type"] = "application/json";
  headers["Content-Length"] = std::to_string(jsonString.length());
  return this;
}

Res* Res::send(const std::string data) {
  this->payload = data;
  headers["Content-Type"] = "text/plain; charset=UTF-8";
  headers["Content-Length"] = std::to_string(data.length());
  return this;
}

Res* Res::setHeader(const std::string key, const std::string value) {
  headers[key] = value;
  return this;
}