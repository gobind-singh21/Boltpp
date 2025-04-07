#pragma once

#include <unordered_map>
#include <string>

#include "json.h"

/**
 * @brief The Res class represents an HTTP response.
 *
 * It contains the status code, payload, protocol, and headers. Methods allow for setting these
 * attributes and for converting JSON responses.
 */
class Res {
  int statusCode = 200;   ///< HTTP status code.
  std::string payload;    ///< Response payload.
  std::string protocol;   ///< HTTP protocol version.

public:
  std::unordered_map<std::string, std::string> headers;  ///< HTTP headers.

  /**
   * @brief Sets the HTTP protocol version.
   *
   * @param protocol The protocol string.
   */
  inline void setProtocol(const std::string protocol) { this->protocol = protocol; }

  /**
   * @brief Gets the HTTP protocol version.
   *
   * @return std::string The protocol string.
   */
  inline std::string getProtocol() const { return protocol; }

  /**
   * @brief Gets the response payload.
   *
   * @return std::string The payload.
   */
  inline std::string getPayload() const { return payload; }

  /**
   * @brief Gets the HTTP status code.
   *
   * @return int The status code.
   */
  inline int getStatusCode() const { return statusCode; }

  /**
   * @brief Sets the status code for the response.
   *
   * @param statusCode The HTTP status code.
   * @return Res* Pointer to the current response (for chaining).
   */
  Res* status(int statusCode);

  /**
   * @brief Sets the response payload as a JSON string.
   *
   * @param j The JSON value to be sent.
   * @return Res* Pointer to the current response.
   */
  Res* json(const JSONValue &j);

  /**
   * @brief Sets the response payload as plain text.
   *
   * @param data The plain text data.
   * @return Res* Pointer to the current response.
   */
  Res* send(const std::string data);

  /**
   * @brief Sets a header for the response.
   *
   * @param key Header name.
   * @param value Header value.
   * @return Res* Pointer to the current response.
   */
  Res* setHeader(const std::string key, const std::string value);

  // Optional destructor cleanup (commented out).
  // ~Res() {
  //   payload.~basic_string();
  //   protocol.~basic_string();
  //   headers.clear();
  // }
};
