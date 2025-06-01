#include <thread>
#include <charconv>
#include <stdexcept>
#include <algorithm>
#include <iostream>

#include "utils.h"
#include "httpserver.h"

void HttpServer::PathTree::addPath(const std::string &path) {
  auto segments = split(path, '/');
  auto node = root;

  for(const auto &segment : segments) {
    if(!segment.empty() && segment[0] == ':') {
      if(!node->paramChild) {
        node->paramChild = std::make_shared<Trie>();
        node->paramChild->paramName = std::string_view(segment.c_str() + 1);
      }
      node = node->paramChild;
    } else {
      if(node->children.find(segment) == node->children.end())
        node->children[segment] = std::make_shared<Trie>();
      node = node->children[segment];
    }
  }

  node->isEndOfPath = true;
}

std::unordered_map<std::string, std::string> HttpServer::PathTree::getPathParams(const std::string &path) {
  auto segments = split(path, '/');
  auto node = root;

  std::unordered_map<std::string, std::string> params;

  for(const auto &segment : segments) {
    if(node->children.find(segment) != node->children.end()) {
      node = node->children[segment];
    } else if(node->paramChild) {
      params[node->paramChild->paramName] = segment;
      node = node->paramChild;
    } else {
      return {};
    }
  }

  if(node->isEndOfPath)
    return params;
  return {};
}

std::string HttpServer::PathTree::getNormalisedPath(const std::string &path) {
  auto segments = split(path, '/');
  auto node = root;

  std::string normalisedPath;
  normalisedPath.reserve(path.size());

  for(const auto &segment : segments) {
    if(node->children.find(segment) != node->children.end()) {
      normalisedPath.push_back('/');
      normalisedPath.append(segment);
      node = node->children[segment];
    } else if(node->paramChild) {
      normalisedPath.append("/:");
      normalisedPath.append(node->paramChild->paramName);
      node = node->paramChild;
    } else {
      return "";
    }
  }

  if(node->isEndOfPath) {
    if(normalisedPath.size() > 1) {
      std::string finalNormalisedPath;
      finalNormalisedPath = std::string_view(normalisedPath.c_str() + 1);
      return finalNormalisedPath;
    }
    return normalisedPath;
  }
  return "";
}

std::string HttpServer::getStatusCodeWord(const int statusCode) {
  switch(statusCode) {
    // 1xx Informational
    case 100: return "Continue";
    case 101: return "Switching Protocols";
    case 102: return "Processing";
    case 103: return "Early Hints";
    // 2xx Successful
    case 200: return "OK";
    case 201: return "Created";
    case 202: return "Accepted";
    case 203: return "Non-Authoritative Information";
    case 204: return "No Content";
    case 205: return "Reset Content";
    case 206: return "Partial Content";
    case 207: return "Multi-Status";
    case 208: return "Already Reported";
    case 226: return "IM Used";
    // 3xx Redirection
    case 300: return "Multiple Choices";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 303: return "See Other";
    case 304: return "Not Modified";
    case 305: return "Use Proxy";
    case 306: return "(Unused)";
    case 307: return "Temporary Redirect";
    case 308: return "Permanent Redirect";
    // 4xx Client Error
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 402: return "Payment Required";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 406: return "Not Acceptable";
    case 407: return "Proxy Authentication Required";
    case 408: return "Request Timeout";
    case 409: return "Conflict";
    case 410: return "Gone";
    case 411: return "Length Required";
    case 412: return "Precondition Failed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 416: return "Range Not Satisfiable";
    case 417: return "Expectation Failed";
    case 421: return "Misdirected Request";
    case 422: return "Unprocessable Entity";
    case 423: return "Locked";
    case 424: return "Failed Dependency";
    case 425: return "Too Early";
    case 426: return "Upgrade Required";
    case 428: return "Precondition Required";
    case 429: return "Too Many Requests";
    case 431: return "Request Header Fields Too Large";
    case 451: return "Unavailable For Legal Reasons";
    // 5xx Server Error
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    case 505: return "HTTP Version Not Supported";
    case 506: return "Variant Also Negotiates";
    case 507: return "Insufficient Storage";
    case 508: return "Loop Detected";
    case 510: return "Not Extended";
    case 511: return "Network Authentication Required";
  }
  return "Not Found";
}

std::string HttpServer::makeHttpResponse(const Response &res) {
  int statusCode = res.getStatusCode();
  const std::string &payload = res.getPayload();

  std::string response;
  response.reserve(10240 + payload.length());

  response.append(res.getProtocol());
  response.push_back(' ');
  response.append(std::to_string(statusCode));
  response.push_back(' ');
  response.append(getStatusCodeWord(statusCode));
  response.append("\r\n");

  if (res.headers.find("Content-Length") == res.headers.end()) {
    response.append("Content-Length: ");
    response.append(std::to_string(payload.size()));
    response.append("\r\n");
  }

  if (res.headers.find("Content-Type") == res.headers.end()) {
    response.append("Content-Type: text/plain; charset=UTF-8\r\n");
  }

  if (res.headers.find("Connection") == res.headers.end()) {
    response.append("Connection: keep-alive\r\n");
  }

  for (const auto &it : res.headers) {
    response.append(it.first);
    response.append(": ");
    response.append(it.second);
    response.append("\r\n");
  }

  response.append("\r\n");
  response.append(payload);
  return response;
}

// char HttpServer::urlEncodingCharacter(std::string_view specialSequence) {
//   if(specialSequence[0] != '%' || specialSequence.length() != 3)
//     return '\0';
//   if(specialSequence.compare("%20") == 0)
//     return ' ';
//   else if(specialSequence.compare("%26") == 0)
//     return '&';
//   else if(specialSequence.compare("%3D") == 0)
//     return '=';
//   else if(specialSequence.compare("%3F") == 0)
//     return '?';
//   else if(specialSequence.compare("%23") == 0)
//     return '#';
//   else if(specialSequence.compare("%25") == 0)
//     return '%';
//   return '\0';
// }

static inline int hexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  return -1;
}

std::string HttpServer::decodeUrl(std::string_view in) {
  std::string out;
  out.reserve(in.size());

  size_t size = in.size();

  for (size_t i = 0; i < size; ++i) {
    char c = in[i];
    if (c == '%') {
      if (i + 2 < in.size()) {
        int hi = hexVal(in[i + 1]);
        int lo = hexVal(in[i + 2]);
        if (hi >= 0 && lo >= 0) {
          out.push_back(static_cast<char>((hi << 4) | lo));
          i += 2;
          continue;
        }
      }
    } else if (c == '+') {
      out.push_back(' ');
      continue;
    }
    out.push_back(c);
  }
  return out;
}

void HttpServer::parseQueryParameters(Request &req) {
  std::string_view url = req.url;
  size_t qmPos = url.find('?');
  
  if (qmPos == std::string_view::npos) {
    req.path = url;
    return;
  }

  req.path = url.substr(0, qmPos);

  std::string_view queryStr = url.substr(qmPos + 1);
  while (!queryStr.empty()) {
    size_t keyEnd = queryStr.find('=');
    size_t pairEnd = queryStr.find('&');

    if (pairEnd == std::string_view::npos)
      pairEnd = queryStr.size();

    std::string_view key, value;

    if (keyEnd != std::string_view::npos && keyEnd < pairEnd) {
      key = queryStr.substr(0, keyEnd);
      value = queryStr.substr(keyEnd + 1, pairEnd - keyEnd - 1);
    } else {
      key = queryStr.substr(0, pairEnd);
      value = "";
    }

    req.queryParameters[decodeUrl(key)] = decodeUrl(value);

    if (pairEnd == queryStr.size()) break;
    queryStr = queryStr.substr(pairEnd + 1);
  }
}

void HttpServer::sendErrorResponse(Response &res, const SOCKET &clientSocket) {
  std::string errorMessage = getStatusCodeWord(res.getStatusCode());
  JSONValue::Object message;
  message["message"] = errorMessage;
  JSONValue jsonMessage(message);
  res.json(jsonMessage).status(res.getStatusCode());
  std::string responseMessage = makeHttpResponse(res);
  send(clientSocket, responseMessage.c_str(), responseMessage.length(), 0);
}

Request HttpServer::sendBadRequest(Request& req, SOCKET clientSocket) {
  Response res;
  res.status(400);
  res.setProtocol(req.protocol.empty() ? "HTTP/1.1" : req.protocol).send("Bad Request");
  sendErrorResponse(res, clientSocket);
  req.payload = "Bad Request";
  return req;
}

Request HttpServer::parseHttpRequest(const std::string &raw, const SOCKET &clientSocket, HttpServer::PathTree &registeredPaths) {
  Request req;
  std::string_view request(raw);
  size_t pos = 0;

  size_t methodEnd = request.find(' ');
  if (methodEnd == std::string_view::npos)
    return sendBadRequest(req, clientSocket);

  req.method = std::string(request.substr(0, methodEnd));

  size_t urlStart = methodEnd + 1;
  size_t urlEnd = request.find(' ', urlStart);
  if (urlEnd == std::string_view::npos)
    return sendBadRequest(req, clientSocket);

  std::string_view fullUrl = request.substr(urlStart, urlEnd - urlStart);
  req.url = std::string(fullUrl);

  size_t protoStart = urlEnd + 1;
  size_t lineEnd = request.find("\r\n", protoStart);
  if (lineEnd == std::string_view::npos)
    return sendBadRequest(req, clientSocket);

  req.protocol = std::string(request.substr(protoStart, lineEnd - protoStart));

  parseQueryParameters(req);
  req.pathParameters = registeredPaths.getPathParams(req.path);

  pos = lineEnd + 2;

  while (pos < request.size()) {
    size_t end = request.find("\r\n", pos);
    if (end == pos) {
      pos += 2;
      break;
    }

    if (end == std::string_view::npos)
      return sendBadRequest(req, clientSocket);

    std::string_view line = request.substr(pos, end - pos);
    size_t colon = line.find(':');
    if (colon == std::string_view::npos)
      return sendBadRequest(req, clientSocket);

    std::string_view key = line.substr(0, colon);
    std::string_view value = line.substr(colon + 1);

    req.headers[trim(key)] = trim(value);
    pos = end + 2;
  }

  if (auto it = req.headers.find("Content-Length"); it != req.headers.end()) {
    int cLength = 0;
    auto [ptr, ec] = std::from_chars(it->second.data(), it->second.data() + it->second.size(), cLength);
    if (ec != std::errc() || ptr != it->second.data() + it->second.size()) {
      Response res;
      res.status(400).send("Malformed request");
      sendErrorResponse(res, clientSocket);
      req.payload = "Bad Request";
      return req;
    }
    req.payload.reserve(cLength);
  }

  // Step 4: Payload
  if (pos < request.size())
    req.payload = raw.substr(pos);

  return req;
}

bool HttpServer::validateCors(Request &req) {
  auto originIt = req.headers.find("Origin");
  if (originIt == req.headers.end())
    return true;

  std::string_view origin = originIt->second;
  bool isWildOrigin = corsConfig.allowedOrigins.find("*") != corsConfig.allowedOrigins.end();
  bool isOriginAllowed = corsConfig.allowedOrigins.find(std::string(origin)) != corsConfig.allowedOrigins.end();

  std::string_view method = req.method;
  bool isMethodAllowed = corsConfig.allowedMethods.find(std::string(method)) != corsConfig.allowedMethods.end();

  if (method == "OPTIONS") {
    auto acrmIt = req.headers.find("Access-Control-Request-Method");
    if (acrmIt != req.headers.end()) {
      std::string_view requestedMethod = acrmIt->second;
      if (corsConfig.allowedMethods.find(std::string(requestedMethod)) == corsConfig.allowedMethods.end()) {
        return false;
      }
    }

    auto acrhIt = req.headers.find("Access-Control-Request-Headers");
    if (acrhIt != req.headers.end()) {
      std::string_view headersStr = acrhIt->second;
      size_t start = 0;
      while (start < headersStr.size()) {
        size_t end = headersStr.find(',', start);
        if (end == std::string_view::npos) end = headersStr.size();

        std::string_view header = headersStr.substr(start, end - start);
        header = trim(header);

        if (!header.empty() &&
            corsConfig.allowedHeaders.find(std::string(header)) == corsConfig.allowedHeaders.end()) {
          return false;
        }

        start = end + 1;
      }
    }
  }

  return (isWildOrigin || isOriginAllowed) && isMethodAllowed;
}

void HttpServer::workerThreadFunction() {
  while (true) {
    RequestPackage task;
    {
      std::unique_lock<std::mutex> lock(queueMutex);
      queueCond.wait(lock, [&]() { return !requestQueue.empty(); });
      task = requestQueue.front();
      requestQueue.pop();
    }
    Request req = parseHttpRequest(task.rawRequest, task.socket, registeredPaths);
    if(req.payload == "Bad Request")
      continue;
    bool isValidRequest = validateCors(req);
    Response res;
    if(isValidRequest) {
      res.setProtocol("HTTP/1.1");
      std::string requestPath = registeredPaths.getNormalisedPath(req.path);
      std::string key = req.method + "::" + registeredPaths.getNormalisedPath(req.path);
      if (requestPath.empty() || allowedRoutes.find(key) == allowedRoutes.end()) {
        res.status(404).send("Not found");
      } else {
        if(req.method == "OPTIONS") {
          res.status(204);
          auto originIt = req.headers.find("Origin");
          if (originIt != req.headers.end()) {
            const std::string &origin = originIt->second;

            if (corsConfig.allowedOrigins.find("*") != corsConfig.allowedOrigins.end()) {
              res.setHeader("Access-Control-Allow-Origin", "*");
            } else if (corsConfig.allowedOrigins.find(origin) != corsConfig.allowedOrigins.end()) {
              res.setHeader("Access-Control-Allow-Origin", origin);
            }

            if (corsConfig.withCredentials) {
              res.setHeader("Access-Control-Allow-Credentials", "true");
            }

            std::string allowedMethods;
            for (const auto &method : corsConfig.allowedMethods) {
              if (!allowedMethods.empty()) allowedMethods.append(", ");
              allowedMethods.append(method);
            }
            res.setHeader("Access-Control-Allow-Methods", allowedMethods);

            std::string allowedHeaders;
            for (const auto &hdr : corsConfig.allowedHeaders) {
              if (!allowedHeaders.empty()) allowedHeaders.append(", ");
              allowedHeaders.append(hdr);
            }
            if (!allowedHeaders.empty()) {
              res.setHeader("Access-Control-Allow-Headers", allowedHeaders);
            }
          }
        } else {
          long long i = 0;
          size_t globalMiddles = globalMiddlewares.size();
          while(i < globalMiddles) {
            globalMiddlewares[i](req, res, i);
            if(i < 0)
              break;
            i++;
          }
          if (i >= 0) {
            i = 0;
            auto &route = allowedRoutes[key];
            size_t routeMiddles = route.middlewares.size();
            while(i < routeMiddles) {
              route.middlewares[i](req, res, i);
              if(i < 0)
                break;
              i++;
            }
            if (i >= 0)
              route.handler(req, res);
          }
        }
      }
    } else {
      res.setProtocol("HTTP/1.1");
      res.status(403);

      res.setHeader("Content-Type", "text/plain");

      if (corsConfig.allowedOrigins.find("*") != corsConfig.allowedOrigins.end())
        res.setHeader("Access-Control-Allow-Origin", "*");
      else if (!corsConfig.allowedOrigins.empty())
        res.setHeader("Access-Control-Allow-Origin", *corsConfig.allowedOrigins.begin());

      if (corsConfig.withCredentials)
        res.setHeader("Access-Control-Allow-Credentials", "true");

      std::string allowedMethods;
      for (const auto& method : corsConfig.allowedMethods) {
        if (!allowedMethods.empty()) allowedMethods.append(", ");
        allowedMethods.append(method);
      }
      res.setHeader("Access-Control-Allow-Methods", allowedMethods);

      std::string allowedHeaders;
      for (const auto& header : corsConfig.allowedHeaders) {
        if (!allowedHeaders.empty()) allowedHeaders.append(", ");
        allowedHeaders.append(header);
      }
      res.setHeader("Access-Control-Allow-Headers", allowedHeaders);

      res.send("CORS Policy Error: Origin or Method or headers not allowed");
    }
    std::string response = makeHttpResponse(res);
    send(task.socket, response.c_str(), response.size(), 0);
    bool terminateSocket = false;
    if (req.headers.find("Connection") != req.headers.end()) {
      std::string connectionHeader = req.headers["Connection"];
      std::transform(connectionHeader.begin(), connectionHeader.end(), connectionHeader.begin(), ::tolower);
      if(connectionHeader.compare("close") == 0)
        terminateSocket = true;
    }
    if(terminateSocket)
      closesocket(task.socket);
    else {
      PerIoData* ioData = new PerIoData();
      ioData->socket = task.socket;
      ioData->wsabuff.buf = ioData->buffer;
      ioData->wsabuff.len = BUFFER_SIZE;
      ioData->receiving = true;
      DWORD flags = 0;
      WSARecv(task.socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
    }
  }
}

void HttpServer::receiverThreadFunction() {
  while (true) {
    DWORD bytesTransfered;
    ULONG_PTR completionKey;
    OVERLAPPED* overlapped;
    BOOL result = GetQueuedCompletionStatus(iocp, &bytesTransfered, &completionKey, &overlapped, INFINITE);
    PerIoData* ioData = reinterpret_cast<PerIoData*>(overlapped);
    if (!result || bytesTransfered == 0) {
      closesocket(ioData->socket);
      socketBuffers.erase(ioData->socket);
      delete ioData;
      continue;
    }
    socketBuffers[ioData->socket].buffer.append(ioData->buffer, bytesTransfered);
    std::string fullBuffer = socketBuffers[ioData->socket].buffer;
    size_t headerEnd = fullBuffer.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
      size_t contentLength = 0;
      size_t pos = fullBuffer.find("Content-Length:");
      if (pos != std::string::npos) {
        pos += contentLengthStringLength;
        while (isspace(fullBuffer[pos])) pos++;
        size_t endPos = fullBuffer.find("\r\n", pos);
        auto [convPtr, ec] = std::from_chars(fullBuffer.c_str() + pos, fullBuffer.c_str() + endPos, contentLength);
        if(convPtr != fullBuffer.c_str() + endPos || ec == std::errc()) {
          Response res;
          res.status(400).send("Content length header invalid");
          sendErrorResponse(res, ioData->socket);
        }
      }
      size_t receivedBodyLength = fullBuffer.size() - (headerEnd + 4);
      if (contentLength == 0 || receivedBodyLength >= contentLength) {
        {
          std::lock_guard<std::mutex> lock(queueMutex);
          requestQueue.push({ ioData->socket, fullBuffer });
        }
        queueCond.notify_one();
        socketBuffers.erase(ioData->socket);
      } else {
        DWORD flags = 0;
        ioData->receiving = true;
        WSARecv(ioData->socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
        continue;
      }
    } else if (fullBuffer.size() < MAX_HEADER_SIZE) {
      DWORD flags = 0;
      ioData->receiving = true;
      WSARecv(ioData->socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
    } else {
      closesocket(ioData->socket);
    }
  }
}

void HttpServer::serverListen() {
  while(true) {
    sockaddr_in clientAddr;
    int addrlen = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrlen);
    if(clientSocket == INVALID_SOCKET)
      continue;
    HANDLE result = CreateIoCompletionPort((HANDLE)clientSocket, iocp, (ULONG_PTR)clientSocket, 0);
    if(result == INVALID_HANDLE_VALUE)
      continue;
    PerIoData* ioData = new PerIoData();
    ioData->socket = clientSocket;
    ioData->wsabuff.buf = ioData->buffer;
    ioData->wsabuff.len = BUFFER_SIZE;
    ioData->receiving = true;
    DWORD flags = 0;
    WSARecv(clientSocket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
  }
}

void HttpServer::setThreads(unsigned int threads) {
  MAX_THREADS = threads;
}

void HttpServer::initServer(int port, std::function<void()> callback = []() {}, int addressFamily = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP) {
  WSADATA wsadata;
  int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if(result != 0) {
    throw std::runtime_error("DLL not found");
  }

  SOCKET initialSocket = socket(addressFamily, type, protocol);
  if(initialSocket == INVALID_SOCKET) {
    WSACleanup();
    throw std::runtime_error(std::string("Error in socket ") + std::to_string(WSAGetLastError()));
  }

  sockaddr_in serverAddr;
  serverAddr.sin_family = addressFamily;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(port);
  int addrSize = sizeof(serverAddr);

  result = bind(initialSocket, (const sockaddr *)&serverAddr, addrSize);
  if(result == SOCKET_ERROR) {
    closesocket(initialSocket);
    WSACleanup();
    throw std::runtime_error(std::string("Error in binding ") + std::to_string(WSAGetLastError()));
  }

  int listenResult = listen(initialSocket, 10);
  if(listenResult == SOCKET_ERROR) {
    closesocket(initialSocket);
    WSACleanup();
    throw std::runtime_error(std::string("Error in listening ") + std::to_string(WSAGetLastError()));
  }

  serverSocket = initialSocket;

  iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
  if(!iocp) {
    throw std::runtime_error("CreateIoCompletionPort failed");
  }

  for(int i = 0; i < MAX_THREADS; i++)
    std::thread(&HttpServer::workerThreadFunction, this).detach();
  
  std::thread(&HttpServer::receiverThreadFunction, this).detach();

  callback();

  serverListen();
}

/**
 * @brief Use this function to create a cors configuration for your backend service
 * @param configurer Lamba function which takes a CorsConfig object by reference as a parameter
 * @throws Runtime error if there is an origin "*" along with withCredentials as true
 */
void HttpServer::createCorsConfig(std::function<void(CorsConfig&)> configurer) {
  configurer(corsConfig);
  if(corsConfig.allowedOrigins.find("*") != corsConfig.allowedOrigins.end() && corsConfig.withCredentials)
    throw std::runtime_error("Can't have \"*\" in allowedOrigins with withCredentials as true");
  corsEnabled = true;
}

void HttpServer::Get(const std::string path, const std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler) {
  std::string key = "GET::";
  key.append(path.c_str(), path.length());
  registeredPaths.addPath(path);
  allowedRoutes[key] = Route(middlewares, handler);
}

void HttpServer::Get(const std::string path, std::function<void(Request&, Response&)> handler) {
  Get(path, {}, handler);
}

void HttpServer::Post(const std::string path, const std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler) {
  std::string key = "POST::";
  key.append(path.c_str(), path.length());
  registeredPaths.addPath(path);
  allowedRoutes[key] = Route(middlewares, handler);
}

void HttpServer::Post(const std::string path, std::function<void(Request&, Response&)> handler) {
  Post(path, {}, handler);
}

void HttpServer::Put(const std::string path, const std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler) {
  std::string key = "PUT::";
  key.append(path.c_str(), path.length());
  registeredPaths.addPath(path);
  allowedRoutes[key] = Route(middlewares, handler);
}

void HttpServer::Put(const std::string path, std::function<void(Request&, Response&)> handler) {
  Put(path, {}, handler);
}

void HttpServer::Patch(const std::string path, const std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler) {
  std::string key = "PATCH::";
  key.append(path.c_str(), path.length());
  registeredPaths.addPath(path);
  allowedRoutes[key] = Route(middlewares, handler);
}

void HttpServer::Patch(const std::string path, std::function<void(Request&, Response&)> handler) {
  Patch(path, {}, handler);
}

void HttpServer::Delete(const std::string path, const std::vector<std::function<void(Request&, Response&, long long&)>> middlewares, std::function<void(Request&, Response&)> handler) {
  std::string key = "DELETE::";
  key.append(path.c_str(), path.length());
  registeredPaths.addPath(path);
  allowedRoutes[key] = Route(middlewares, handler);
}

void HttpServer::Delete(const std::string path, std::function<void(Request&, Response&)> handler) {
  Delete(path, {}, handler);
}
