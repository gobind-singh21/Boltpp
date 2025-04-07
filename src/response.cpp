#include "response.h"

/**
 * @brief Sets the HTTP status code of the response.
 *
 * @param statusCode The desired status code.
 * @return Res* Pointer to the current response for chaining.
 */
Res* Res::status(int statusCode) {
  this->statusCode = statusCode;
  return this;
}

/**
 * @brief Converts a JSONValue to a JSON string and sets it as the response payload.
 *
 * Also sets the Content-Type header to "application/json".
 *
 * @param j The JSON value.
 * @return Res* Pointer to the current response for chaining.
 */
Res* Res::json(const JSONValue &j) {
  std::string jsonString = j.stringify();
  this->payload = jsonString;
  headers["Content-Type"] = "application/json";
  headers["Content-Length"] = std::to_string(jsonString.length());
  return this;
}

/**
 * @brief Sets a plain text payload for the response.
 *
 * Also sets the Content-Type header to "text/plain; charset=UTF-8".
 *
 * @param data The plain text data.
 * @return Res* Pointer to the current response for chaining.
 */
Res* Res::send(const std::string data) {
  this->payload = data;
  headers["Content-Type"] = "text/plain; charset=UTF-8";
  headers["Content-Length"] = std::to_string(data.length());
  return this;
}

/**
 * @brief Sets a header field for the response.
 *
 * @param key The header name.
 * @param value The header value.
 * @return Res* Pointer to the current response for chaining.
 */
Res* Res::setHeader(const std::string key, const std::string value) {
  headers[key] = value;
  return this;
}
