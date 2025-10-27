#pragma once

#include <unordered_map>
#include <string>

#include "json.h"

/**
 * @brief The Response class represents an HTTP response.
 *
 * It contains the status code, payload, protocol, and headers. Methods allow for setting these
 * attributes and for converting JSON responses.
 */
class Response {
  int statusCode = 200;   ///< HTTP status code.
  std::string payload;    ///< Response payload.
  std::string protocol = "HTTP/1.1";   ///< HTTP protocol version.

public:
  Response() {
    headers["Content-Type"] = "text/plain; charset=UTF-8";
  }

  std::unordered_map<std::string, std::string> headers;  ///< HTTP headers.

  /**
   * @brief Sets the HTTP protocol version.
   *
   * @param protocol The protocol string.
   */
  Response& setProtocol(const std::string protocol);

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
   * @return Response reference to the current response (for chaining).
   */
  Response& status(int statusCode);

  /**
   * @brief Sets the response payload as a JSON string.
   *
   * @param j The JSON value to be sent.
   * @return Response reference to the current response.
   */
  Response& json(const JSONValue &j);

  // /**
  //  * @brief Sets the response payload as plain text.
  //  *
  //  * @param data The plain text data.
  //  * @return Response reference to the current response.
  //  */
  // Response& send(const std::string data);
  
  /**
   * @brief Sets the response payload as plain text (recommended).
   *
   * @param data string of the data to be inserted.
   * @return Response reference to the current response.
   */
  Response& send(const std::string_view dataView);

  /**
   * @brief Sets a header for the response.
   *
   * @param key Header name.
   * @param value Header value.
   * @return Response reference to the current response.
   */
  Response& setHeader(const std::string_view key, const std::string_view value);
};