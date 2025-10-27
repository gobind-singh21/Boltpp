#include "response.h"

/**
 * @brief Sets the HTTP status code of the response.
 *
 * @param statusCode The desired status code.
 * @return Response* Pointer to the current response for chaining.
 */

Response& Response::setProtocol(const std::string protocol) {
  this->protocol = protocol;
  return *(this);
}

Response& Response::status(int statusCode) {
  this->statusCode = statusCode;
  return *this;
}

/**
 * @brief Converts a JSONValue to a JSON string and sets it as the response payload.
 *
 * Also sets the Content-Type header to "application/json".
 *
 * @param j The JSON value.
 * @return Response* Pointer to the current response for chaining.
 */
Response& Response::json(const JSONValue &j) {
  std::string jsonString = j.stringify();
  this->payload = jsonString;
  headers["Content-Type"] = "application/json";
  headers["Content-Length"] = std::to_string(jsonString.length());
  return *this;
}

Response& Response::send(const std::string_view dataView) {
  this->payload = dataView;
  headers["Content-Length"] = std::to_string(dataView.length());
  return *this;
}

/**
 * @brief Sets a header field for the response.
 *
 * @param key The header name.
 * @param value The header value.
 * @return Response* Pointer to the current response for chaining.
 */
Response& Response::setHeader(const std::string_view key, const std::string_view value) {
  headers[std::string(key)] = value;
  return *this;
}