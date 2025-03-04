#include <iostream>
#include <thread>
#include <charconv>

#include "utils.h"
#include "httpserver.h"

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

char HttpServer::urlEncodingCharacter(const std::string specialSequence) {
  if(specialSequence[0] != '%' || specialSequence.length() != 3)
    return '\0';
  if(specialSequence.compare("%20") == 0) {
    return ' ';
  } else if(specialSequence.compare("%26") == 0) {
    return '&';
  } else if(specialSequence.compare("%3D") == 0) {
    return '=';
  } else if(specialSequence.compare("%3F") == 0) {
    return '?';
  } else if(specialSequence.compare("%23") == 0) {
    return '#';
  } else if(specialSequence.compare("%25") == 0) {
    return '%';
  }
  return '\0';
}

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

void HttpServer::sendErrorResponse(Res &res, const SOCKET &clientSocket) {
  std::string errorMessage = getStatusCodeWord(res.getStatusCode());
  JSONValue::Object message;
  message["message"] = errorMessage;
  JSONValue jsonMessage(message);
  res.json(jsonMessage)->status(res.getStatusCode());
  std::string responseMessage = makeHttpResponse(res);
  send(clientSocket, responseMessage.c_str(), responseMessage.length(), 0);
}

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
  bool keyEnd = false;
  while(i < size) {
    char c = request[i];
    if(c == ':') {
      keyEnd = true;
    } else if(i < thirdLastIndex && c == '\r' && request[i + 1] == '\n' && request[i + 2] == '\r' && request[i + 3] == '\n') {
      if(key != "" && value != "") {
        req.headers[trim(key)] = trim(value);
        key = value = "";
      } else {
        Res res;
        res.status(400)->setProtocol(req.protocol);
        sendErrorResponse(res, clientSocket);
        req.payload = "Bad request";
        return req;
      }
      break;
    } else if(i < lastIndex && c == '\r' && request[i + 1] == '\n') {
      keyEnd = false;
      req.headers[trim(key)] = trim(value);
      key = value = "";
    } else {
      if(keyEnd)
        value.push_back(c);
      else
        key.push_back(c);
    }
    i++;
  }
  if(key != "" || value != "") {
    Res res;
    res.status(400)->setProtocol(req.protocol);
    sendErrorResponse(res, clientSocket);
    req.payload = "Bad Request";
    return req;
  }
  i += 4;

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

void HttpServer::handleClientRequest(const SOCKET &clientSocket) {
  char recvBuffer[10240];
  int bytesRecieved = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
  
  if(bytesRecieved > 0) {
    recvBuffer[bytesRecieved] = '\0';
    std::string request(recvBuffer);
    
    Res res;
    Req req = parseHttpRequest(request, clientSocket);
    if(req.payload == "Bad Request") {
      closesocket(clientSocket);
      return;
    }
    res.setProtocol(req.protocol);
    if(req.headers.find("Content-Length") != req.headers.end()) {
      int contentLength = std::stoi(req.headers["Content-Length"]);
      std::string payload = "";
      int totalRecieved = request.size() - request.find("\r\n\r\n") - 4;
      if(totalRecieved < contentLength) {
        payload.append(request.substr(request.find("\r\n\r\n") + 4));
        while(totalRecieved < contentLength) {
          int remaining = std::min(4096, contentLength - totalRecieved);
          int recieved = recv(clientSocket, recvBuffer, remaining, 0);
          if(recieved <= 0)
            break;
          payload.append(recvBuffer, recieved);
          totalRecieved += recieved;
        }
      }
      if(payload != "")
        req.payload = payload;
    }
    std::string key = req.method + "::" + req.path;
    
    if(allowed.find(key) == allowed.end())
      res.status(404)->send("Not found");
    else {
      long long i = 0, size = globalMiddlewares.size();
      while(i < size) {
        globalMiddlewares[i](req, res, i);
        if(i < 0)
          break;
      }
      if(i < 0) {
        sendErrorResponse(res, clientSocket);
        closesocket(clientSocket);
        return;
      }
      i = 0;
      size = allowed[key].middlewares.size();
      while(i < size) {
        allowed[key].middlewares[i](req, res, i);
        if(i < 0)
          break;
      }
      if(i < 0) {
        sendErrorResponse(res, clientSocket);
        closesocket(clientSocket);
        return;
      }
      allowed[key].handler(req, res);
    }
    
    std::string httpResponse = makeHttpResponse(res);
    send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);
    // std::cout << "Response sent" << std::endl;
  
  } else if(bytesRecieved == 0) {
    std::cerr << "Client disconnected" << std::endl;
    closesocket(clientSocket);
    // std::cout << "Client connection closed" << std::endl;
  } else {
    std::cerr << "Error in receiving from client" << std::endl;
    closesocket(clientSocket);
    // std::cout << "Client connection closed" << std::endl;
  }
  
  closesocket(clientSocket);
}

void HttpServer::serverListen(const SOCKET &serverSocket) {
  while(true) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrSize);
    if(clientSocket == INVALID_SOCKET) {
      std::cerr << "Accepting client failed: " << WSAGetLastError() << std::endl;
      continue;
    }
    std::thread clientThread(&HttpServer::handleClientRequest, this, clientSocket);
    clientThread.detach();
  }
}

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
  std::thread connectionThread(&HttpServer::serverListen, this, serverSocket);
  connectionThread.detach();
  return initialSocket;
}

void HttpServer::Get(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "GET::" + path;
  allowed[key] = Route(middlewares, handler);
}

void HttpServer::Post(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "POST::" + path;
  allowed[key] = Route(middlewares, handler);
}

void HttpServer::Put(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "PUT::" + path;
  allowed[key] = Route(middlewares, handler);
}

void HttpServer::Patch(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "PATCH::" + path;
  allowed[key] = Route(middlewares, handler);
}

void HttpServer::Delete(const std::string path, const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares, std::function<void(Req&, Res&)> handler) {
  std::string key = "DELETE::" + path;
  allowed[key] = Route(middlewares, handler);
}