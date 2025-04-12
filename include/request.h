#pragma once

#include <unordered_map>
#include <string>

#include "json.h"

/**
 * @brief The Req class represents an HTTP request.
 *
 * Contains HTTP method, path, protocol version, payload, query parameters, headers, and body.
 */
class Req {
public:
  /**
   * @brief Default constructor initializes empty values.
   */
  Req() : method(""), path(""), protocol(""), payload(""), queryParameters({}), headers({}) {}

  /**
   * @brief Parameterized constructor for initializing a request.
   *
   * @param _method HTTP method (GET, POST, etc.).
   * @param _path URL path.
   * @param _protocol HTTP protocol version.
   * @param _payload Request payload.
   * @param query Map of query parameters.
   * @param _headers Map of HTTP headers.
   */
  Req(std::string _method, std::string _path, std::string _protocol, std::string _payload,
      std::unordered_map<std::string, std::string> query,
      std::unordered_map<std::string, std::string> _headers)
      : method(_method), path(_path), protocol(_protocol), payload(_payload),
        queryParameters(query), headers(_headers) {}

  /**
   * @brief Copy constructor.
   *
   * @param req The request object to copy.
   */
  Req(const Req &req)
      : method(req.method), path(req.path), protocol(req.protocol), payload(req.payload),
        queryParameters(req.queryParameters), headers(req.headers) {}

  std::string method;  ///< HTTP method.
  std::string path;    ///< URL path.
  std::string protocol = "HTTP/1.1";  ///< HTTP protocol version.
  std::string payload; ///< Raw request payload.
  std::unordered_map<std::string, std::string> queryParameters;  ///< Query parameters from URL.
  std::unordered_map<std::string, std::string> headers;  ///< HTTP headers.
  JSONValue body;      ///< Parsed JSON body (if applicable).
};