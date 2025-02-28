#pragma once

#include <winsock2.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <functional>
#include <vector>
#include <utility>
#include <unordered_map>
#include <sstream>
#include <thread>

#include "utils.h"
#include "request.h"
#include "response.h"

#pragma comment(lib, "ws2_32.lib")

class HttpServer {
  SOCKET serverSocket;
  class Route {
  public:
    Route() : middlewares({}), handler([](Req &req, Res &res) {}) {}
    Route(std::vector<std::function<void(Req&, Res&, size_t&)>> _middlewares,
          std::function<void(Req&, Res&)> _handler)
          : middlewares(_middlewares), handler(_handler) {}
    Route(const Route &route) : middlewares(route.middlewares), handler(route.handler) {}
  
    std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares;
    std::function<void(Req&, Res&)> handler;
  };

  std::unordered_map<std::string, Route> allowed;
  std::vector<std::function<void(Req&, Res&, size_t&)>> globalMiddlewares;

  // Private static helper functions.
  static std::string getStatusCodeWord(const int statusCode) {
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

  static std::string makeHttpResponse(const Res &res) {
    int statusCode = res.getStatusCode();
    std::string response = res.getProtocol() + " " + std::to_string(statusCode) + " " +
                           getStatusCodeWord(statusCode) + "\r\n";
    for(const auto &it : res.headers)
      response += it.first + ": " + it.second + "\r\n";
    response += "\r\n" + res.getPayload() + "\r\n";
    return response;
  }

  static std::unordered_map<std::string, std::string> getQueryParameters(const std::string &queryString) {
    size_t size = queryString.length();
    std::unordered_map<std::string, std::string> res;
    std::string key = "", value = "";
    bool keyEnd = false;
    for(size_t i = 0; i < size; i++) {
      if(queryString[i] == '&') {
        keyEnd = false;
        if(key != "")
          res[key] = value;
        key = value = "";
      }
      if(queryString[i] == '=')
        keyEnd = true;
      if(keyEnd)
        value += queryString[i];
      else
        key += queryString[i];
    }
    if(key != "")
      res[key] = value;
    return res;
  }

  static std::pair<std::string, std::string> getHeaderKeyValue(const std::string &header) {
    std::string key = "", value = "";
    bool keyComplete = false;
    for (const char &ch : header) {
      if(ch == ':')
        keyComplete = true;
      else if(!keyComplete)
        key += ch;
      else
        value += ch;
    }
    return {trim(key), trim(value)};
  }

  static Req parseHttpRequest(const std::string &request) {
    std::stringstream ss(request);
    std::string line;
    getline(ss, line);
    std::vector<std::string> firstLine = split(line, ' ');
    std::string method = firstLine[0], path = firstLine[1], protocol = firstLine[2];
    protocol.pop_back();

    std::unordered_map<std::string, std::string> headers, queryParameters;
    int queryParamPos = path.find("?");
    if(queryParamPos != std::string::npos)
      queryParameters = getQueryParameters(path.substr(queryParamPos, path.length() - queryParamPos - 1));
    bool headerEnd = false;
    std::string payload = "";

    while(getline(ss, line)) {
      if(headerEnd) {
        if(line.back() == '\r')
          line.pop_back();
        line += '\n';
        payload += line;
        continue;
      }
      if(line == "\r" || line == "\n" || line == "\n\r")
        headerEnd = true;
      std::pair<std::string, std::string> headerKeyValue = getHeaderKeyValue(line);
      headers.insert(headerKeyValue);
    }
    return Req(method, path, protocol, payload, queryParameters, headers);
  }

  static void sendErrorResponse(Res &res, const SOCKET clientSocket, int statusCode) {
    std::string errorMessage = getStatusCodeWord(statusCode);
    JSONValue::Object message;
    message["message"] = errorMessage;
    JSONValue jsonMessage(message);
    res.json(jsonMessage)->status(statusCode);
    std::string responseMessage = makeHttpResponse(res);
    send(clientSocket, responseMessage.c_str(), responseMessage.length(), 0);
  }

  // Private member functions to handle client connections.
  void handleClientRequest(const SOCKET &clientSocket) {
    char recvBuffer[10240];
    int bytesRecieved = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
    
    if(bytesRecieved > 0) {
      recvBuffer[bytesRecieved] = '\0';

      std::string request(recvBuffer);
      
      Req req = parseHttpRequest(request);
      Res res;
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
        size_t i = 0, size = globalMiddlewares.size();
        while(i < size)
          globalMiddlewares[i](req, res, i);
        i = 0;
        size = allowed[key].middlewares.size();
        while(i < size)
          allowed[key].middlewares[i](req, res, i);
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

  void serverListen(const SOCKET serverSocket) {
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
      // handleClientRequest(clientSocket);
    }
  }

public:
  HttpServer() : serverSocket(INVALID_SOCKET) {}
  HttpServer(const SOCKET s) : serverSocket(s) {}

  int initServer(int addressFamily, int type, int protocol, int port) {
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

  void use(std::function<void(Req&, Res&, size_t&)> middleware) {
    globalMiddlewares.emplace_back(middleware);
  }

  // Route registration functions.
  void Get(const std::string path,
           const std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares,
           std::function<void(Req&, Res&)> handler) {
    std::string key = "GET::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Post(const std::string path,
            const std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares,
            std::function<void(Req&, Res&)> handler) {
    std::string key = "POST::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Patch(const std::string path,
             const std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares,
             std::function<void(Req&, Res&)> handler) {
    std::string key = "PATCH::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Put(const std::string path,
           const std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares,
           std::function<void(Req&, Res&)> handler) {
    std::string key = "PUT::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Delete(const std::string path,
              const std::vector<std::function<void(Req&, Res&, size_t&)>> middlewares,
              std::function<void(Req&, Res&)> handler) {
    std::string key = "DELETE::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  ~HttpServer() {
    closesocket(serverSocket);
    WSACleanup();
  }
};