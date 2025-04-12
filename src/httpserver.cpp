#include <iostream>
#include <thread>
#include <charconv>
#include <stdexcept>

#include "utils.h"
#include "httpserver.h"

// Define the thread-local storage for socket buffers.
thread_local std::unordered_map<SOCKET, HttpServer::SocketBuffer> HttpServer::socketBuffers;

/**
 * @brief Returns the textual description for a given HTTP status code.
 *
 * @param statusCode The HTTP status code.
 * @return std::string The corresponding status message.
 */
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

/**
 * @brief Constructs an HTTP response string from the given Res object.
 *
 * @param res The response object.
 * @return std::string The full HTTP response string.
 */
std::string HttpServer::makeHttpResponse(const Res &res) {
  int statusCode = res.getStatusCode();
  std::string response, payload = res.getPayload();
  response.reserve(10240 + payload.length());
  response.append(res.getProtocol());
  response.append(" ");
  response.append(std::to_string(statusCode));
  response.append(" ");
  response.append(getStatusCodeWord(statusCode));
  response.append("\r\n");
  for(const auto &it : res.headers) {
    response.append(it.first);
    response.append(": ");
    response.append(it.second);
    response.append("\r\n");
  }
  response.append("\r\n");
  response.append(payload);
  response.append("\r\n");
  return response;
}

/**
 * @brief Decodes a URL-encoded sequence into its corresponding character.
 *
 * @param specialSequence The URL-encoded sequence.
 * @return char The decoded character or '\0' if invalid.
 */
char HttpServer::urlEncodingCharacter(const std::string specialSequence) {
  if(specialSequence[0] != '%' || specialSequence.length() != 3)
    return '\0';
  if(specialSequence.compare("%20") == 0)
    return ' ';
  else if(specialSequence.compare("%26") == 0)
    return '&';
  else if(specialSequence.compare("%3D") == 0)
    return '=';
  else if(specialSequence.compare("%3F") == 0)
    return '?';
  else if(specialSequence.compare("%23") == 0)
    return '#';
  else if(specialSequence.compare("%25") == 0)
    return '%';
  return '\0';
}

/**
 * @brief Parses query parameters from the URL and stores them in the Req object.
 *
 * @param req The request object.
 */
void HttpServer::parseQueryParameters(Req &req) {
  size_t pathSize = req.path.length(), lastIndex = pathSize - 1, pos = -1;
  for(size_t i = 0; i < pathSize; i++) {
    if(req.path[i] == '?') {
      pos = i;
      break;
    }
  }
  if(pos == -1 || pos == lastIndex)
    return;
  pos++;
  bool keyEnd = false;
  size_t thirdLast = pathSize - 3;
  std::string key = "", value = "";
  for(size_t i = pos; i < pathSize; i++) {
    char c = req.path[i];
    if(c == '&') {
      req.queryParameters[key] = value;
    } else if(c == '=') {
      keyEnd = true;
    } else if(c == '%' && i < thirdLast) {
      char ch = urlEncodingCharacter(req.path.substr(i, 3));
      if(keyEnd == false)
        key.push_back(ch);
      else
        value.push_back(ch);
      i += 2;
    } else {
      if(keyEnd == false)
        key.push_back(c);
      else
        value.push_back(c);
    }
  }
  if(key != "")
    req.queryParameters[key] = value;
}

/**
 * @brief Sends an error response back to the client.
 *
 * Constructs a JSON error message and sends it over the socket.
 *
 * @param res The response object containing the error status.
 * @param clientSocket The client socket.
 */
void HttpServer::sendErrorResponse(Res &res, const SOCKET &clientSocket) {
  std::string errorMessage = getStatusCodeWord(res.getStatusCode());
  JSONValue::Object message;
  message["message"] = errorMessage;
  JSONValue jsonMessage(message);
  res.json(jsonMessage)->status(res.getStatusCode());
  std::string responseMessage = makeHttpResponse(res);
  send(clientSocket, responseMessage.c_str(), responseMessage.length(), 0);
}

/**
 * @brief Parses a raw HTTP request string and returns a Req object.
 *
 * @param request The raw HTTP request.
 * @param clientSocket The client socket.
 * @return Req The parsed request.
 */
Req HttpServer::parseHttpRequest(const std::string &request, const SOCKET &clientSocket) {
  Req req;
  size_t size = request.size(), i = 0, lastIndex = size - 1, thirdLastIndex = size - 4;
  short token = 0;
  while(i < size && !(request[i] == '\r' && i < lastIndex && request[i + 1] == '\n')) {
    char c = request[i];
    if(c == ' ')
      token++;
    else {
      if(token == 0)
        req.method.push_back(c);
      else if(token == 1)
        req.path.push_back(c);
      else if(token == 2)
        req.protocol.push_back(c);
    }
    i++;
  }
  i += 2;

  std::string key = "", value = "";
  key.reserve(200);
  value.reserve(1024);
  while(i < size) {
    while(request[i] != ':') {
      key.push_back(request[i]);
      i++;
    }
    i++;
    while(i < lastIndex && request[i] != '\r' && request[i + 1] != '\n') {
      value.push_back(request[i]);
      i++;
    }
    req.headers[trim(key)] = trim(value);
    key = value = "";
    i += 2;
    if(i < lastIndex && request[i] == '\r' && request[i + 1] == '\n')
      break;
  }
  if(key != "" || value != "") {
    Res res;
    res.status(400)->setProtocol(req.protocol);
    sendErrorResponse(res, clientSocket);
    req.payload = "Bad Request";
    return req;
  }

  if(req.headers.find("Content-Length") != req.headers.end()) {
    std::string contentLength = req.headers["Content-Length"];
    int cLength = 0, length = contentLength.length();
    auto res = std::from_chars(contentLength.data(), contentLength.data() + length, cLength);
    if(res.ec != std::errc() || (res.ptr != contentLength.data() + length)) {
      Res res;
      res.status(400)->send("Malformed request");
      sendErrorResponse(res, clientSocket);
      req.payload = "Bad Request";
      return req;
    } else {
      req.payload.reserve(cLength);
    }
  }

  while(i < size)
    req.payload.push_back(request[i++]);
  return req;
}

/**
 * @brief The worker thread function that processes IO completion events.
 *
 * Waits for IO events, reads the data from the socket buffers,
 * parses requests, and dispatches them to the correct route handlers.
 */
void HttpServer::workerThread() {
  while(true) {
    DWORD bytesTransfered;
    ULONG_PTR completionKey;
    OVERLAPPED* overlapped;
    BOOL result = GetQueuedCompletionStatus(iocp, &bytesTransfered, &completionKey, &overlapped, INFINITE);
    PerIoData* ioData = reinterpret_cast<PerIoData*>(overlapped);
    if(!result || bytesTransfered == 0) {
      closesocket(ioData->socket);
      {
        std::lock_guard<std::mutex> lock(socketBuffers[ioData->socket].mtx);
        socketBuffers.erase(ioData->socket);
      }
      delete ioData;
      continue;
    }
    if(ioData->receiving) {
      {
        std::lock_guard<std::mutex> lock(socketBuffers[ioData->socket].mtx);
        socketBuffers[ioData->socket].buffer.append(ioData->buffer, bytesTransfered);
      }
      bool shouldProcess = false;
      std::string localBuffer;
      {
        std::lock_guard<std::mutex> lock(socketBuffers[ioData->socket].mtx);
        if(!socketBuffers[ioData->socket].processing && socketBuffers[ioData->socket].buffer.find("\r\n\r\n") != std::string::npos) {
          localBuffer = socketBuffers[ioData->socket].buffer;
          socketBuffers[ioData->socket].processing = true;
          shouldProcess = true;
        }
      }

      if(!shouldProcess) {
        DWORD flags = 0;
        WSARecv(ioData->socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
        continue;
      }

      size_t headerEnd = localBuffer.find("\r\n\r\n"), headerSize = localBuffer.length();

      if(headerEnd != std::string::npos) {
        int expectedContentLength = 0;
        size_t pos = localBuffer.find("Content-Length");
        if(pos != std::string::npos) {
          pos += strlen("Content-Length:");
          while(pos < headerEnd && isspace(localBuffer[pos])) { pos++; }
          size_t endPos = localBuffer.find("\r\n", pos);
          try {
            expectedContentLength = std::stoi(localBuffer.substr(pos, endPos - pos));
          } catch (...) {
            expectedContentLength = 0;
          }
        }
        size_t receivedBodyLength = localBuffer.size() - (headerEnd + 4);
        if(expectedContentLength == 0 || receivedBodyLength >= (size_t)expectedContentLength) {
          Req req = parseHttpRequest(localBuffer, ioData->socket);
          {
            std::lock_guard<std::mutex> lock(socketBuffers[ioData->socket].mtx);
            socketBuffers.erase(ioData->socket);
          }
          localBuffer.clear();
          Res res;
          res.setProtocol("HTTP/1.1");
          std::string key = req.method + "::" + req.path;
          if(allowedRoutes.find(key) == allowedRoutes.end()) {
            res.status(404)->send("Not found");
          } else {
            long long i = 0;
            size_t mwSize = globalMiddlewares.size();
            while(i < mwSize && i >= 0) {
              globalMiddlewares[i](req, res, i);
              i++;
            }
            if(i >= 0) {
              i = 0;
              mwSize = allowedRoutes[key].middlewares.size();
              while(i < mwSize && i >= 0) {
                allowedRoutes[key].middlewares[i](req, res, i);
                i++;
              }
              if(i >= 0)
                allowedRoutes[key].handler(req, res);
            }
            std::string httpResponse = makeHttpResponse(res);
            ioData->wsabuff.buf = ioData->buffer;
            ioData->wsabuff.len = httpResponse.size();
            memcpy(ioData->buffer, httpResponse.c_str(), httpResponse.size());
            ioData->receiving = false;
            WSASend(ioData->socket, &ioData->wsabuff, 1, nullptr, 0, &ioData->overlapped, nullptr);

            if(req.headers.find("Connection") != req.headers.end()) {
              size_t pos = req.headers["Connection"].find("keep-alive");
              if(pos == std::string::npos) {
                if(req.headers["Connection"] == "close")
                  closesocket(ioData->socket);
              } else {
                // TODO : implement keep alive header functionality
              }
            } else
              closesocket(ioData->socket);
          }
        } else {
          ioData->receiving = true;
          DWORD flags = 0;
          WSARecv(ioData->socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
          continue;
        }
      } else if(headerSize < MAX_HEADER_SIZE) {
        ioData->receiving = true;
        DWORD flags = 0;
        WSARecv(ioData->socket, &ioData->wsabuff, 1, nullptr, &flags, &ioData->overlapped, nullptr);
        continue;
      } else {
        socketBuffers.erase(ioData->socket);
        Res res;
        res.setProtocol("HTTP/1.1");
        res.status(400)->send("Header size exceeded");
        std::string response = makeHttpResponse(res);
        ioData->wsabuff.buf = ioData->buffer;
        ioData->wsabuff.len = response.size();
        memcpy(ioData->buffer, response.c_str(), response.size());
        WSASend(ioData->socket, &ioData->wsabuff, 1, nullptr, 0, &ioData->overlapped, nullptr);
        closesocket(ioData->socket);
        delete ioData;
        continue;
      }
    } else {
      delete ioData;
    }
  }
}

/**
 * @brief Accepts incoming client connections and sets up asynchronous IO.
 *
 * For each accepted connection, associates the client socket with the IO Completion Port and
 * starts an asynchronous receive.
 *
 * @param serverSocket The server socket.
 */
void HttpServer::serverListen(const SOCKET &serverSocket) {
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

/**
 * @brief Initializes the server socket, binds to the port, and starts listening.
 *
 * Also initializes the IO Completion Port and spawns worker threads.
 *
 * @param addressFamily The address family (e.g., AF_INET).
 * @param type The socket type (e.g., SOCK_STREAM).
 * @param protocol The protocol (e.g., IPPROTO_TCP).
 * @param port The port number.
 * @return int The server socket descriptor.
 */
int HttpServer::initServer(int addressFamily, int type, int protocol, int port) {
  WSADATA wsadata;
  int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
  if(result != 0) {
    throw std::runtime_error("DLL not found");
    return result;
  }

  SOCKET initialSocket = socket(addressFamily, type, protocol);
  if(initialSocket == INVALID_SOCKET) {
    WSACleanup();
    throw std::runtime_error(std::string("Error in socket ") + std::to_string(WSAGetLastError()));
    return initialSocket;
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
    return result;
  }

  int listenResult = listen(initialSocket, 10);
  if(listenResult == SOCKET_ERROR) {
    closesocket(initialSocket);
    WSACleanup();
    throw std::runtime_error(std::string("Error in listening ") + std::to_string(WSAGetLastError()));
    return listenResult;
  }

  serverSocket = initialSocket;
  std::cout << "Server listening on port " << port << std::endl;

  iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  if(!iocp) {
    throw std::runtime_error("CreateIoCompletionPort failed");
  }

  for(int i = 0; i < MAX_THREADS; i++) {
    workerThreads.emplace_back(&HttpServer::workerThread, this);
  }

  return initialSocket;
}

/**
 * @brief Registers a GET route.
 *
 * @param path The URL path.
 * @param middlewares Vector of middlewares for this route.
 * @param handler The handler function for the route.
 */
void HttpServer::Get(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "GET::" + path;
  allowedRoutes[key] = Route(middlewares, handler);
}

/**
 * @brief Registers a POST route.
 *
 * @param path The URL path.
 * @param middlewares Vector of middlewares for this route.
 * @param handler The handler function for the route.
 */
void HttpServer::Post(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "POST::" + path;
  allowedRoutes[key] = Route(middlewares, handler);
}

/**
 * @brief Registers a PUT route.
 *
 * @param path The URL path.
 * @param middlewares Vector of middlewares for this route.
 * @param handler The handler function for the route.
 */
void HttpServer::Put(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "PUT::" + path;
  allowedRoutes[key] = Route(middlewares, handler);
}

/**
 * @brief Registers a PATCH route.
 *
 * @param path The URL path.
 * @param middlewares Vector of middlewares for this route.
 * @param handler The handler function for the route.
 */
void HttpServer::Patch(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "PATCH::" + path;
  allowedRoutes[key] = Route(middlewares, handler);
}

/**
 * @brief Registers a DELETE route.
 *
 * @param path The URL path.
 * @param middlewares Vector of middlewares for this route.
 * @param handler The handler function for the route.
 */
void HttpServer::Delete(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "DELETE::" + path;
  allowedRoutes[key] = Route(middlewares, handler);
}