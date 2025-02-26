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

#pragma comment(lib, "ws2_32.lib")

static std::string getStatusCodeWord(const int statusCode) {
  switch(statusCode) {
    case 200:
      return "OK";
    case 201:
      return "Created";
    case 202:
      return "Accepted";
  }
  return "Not Found";
}

// Request object class
class Req {
  public:
  
  Req() : method(""), path(""), queryParameters({}), headers({}) {}
  Req(std::string _method, std::string _path, std::string _protocol, std::unordered_map<std::string, std::string> query, std::unordered_map<std::string, std::string> _headers) : method(_method), path(_path), protocol(_protocol), queryParameters(query), headers(_headers) {}
  Req(const Req &req) : method(req.method), path(req.path), protocol(req.protocol), queryParameters(req.queryParameters), headers(req.headers) {}
  
  std::string method;
  std::string path;
  std::string protocol = "HTTP/1.1";
  std::unordered_map<std::string, std::string> queryParameters;
  std::unordered_map<std::string, std::string> headers;
};

// Response object class
class Res {
  
  int statusCode = 200;
  std::string payload;
  std::string protocol;
  
public:
  
  std::unordered_map<std::string, std::string> headers;
  
  inline void setProtocol(const std::string protocol) { this->protocol = protocol; }
  inline std::string getProtocol() const { return protocol; }
  inline std::string getPayload() const { return payload; }
  inline int getStatusCode() const { return statusCode; }
  
  // Set status code of response object
  Res* status(int statusCode) {
    this->statusCode = statusCode;
    return this;
  }
  
  // Pass unordered map
  // and set that map as payload
  // in json format
  Res* json(const std::unordered_map<std::string, std::string> &map) {
    std::string res = "{";
    for(auto &it : map)
    res += "\"" + it.first + "\"" + ":" + "\"" + it.second + "\",";
    res.pop_back();
    res += '}';
    this->payload = res;
    headers["Content-Type"] = "application/json";
    headers["Content-Length"] = std::to_string(res.length());
    return this;
  }
  
  // Pass string as data
  // and set that as a payload
  Res* send(std::string data) {
    this->payload = data;
    headers["Content-Type"] = "text/plain; charset=UTF-8";
    headers["Content-Length"] = std::to_string(data.length());
    return this;
  }
  
  Res* setHeader(const std::string key, const std::string value) {
    headers[key] = value;
    return this;
  }
};

static std::string makeHttpResponse(Res &res) {
  int statusCode = res.getStatusCode();
  std::string response = res.getProtocol() + " " + std::to_string(statusCode) + " " + getStatusCodeWord(statusCode) + "\r\n";
  // std::cout << response << std::endl;
  for(auto &it : res.headers)
    response += it.first + ": " + it.second + "\r\n";
  response += "\r\n";
  response += res.getPayload() + "\r\n";
  return response;
}

static std::unordered_map<std::string, std::string> getQueryParameters(const std::string queryString) {
  std::stringstream ss(queryString);
  std::string temp;
  std::unordered_map<std::string, std::string> res;
  while(getline(ss, temp, '&')) {
    std::vector<std::string> query = split(temp, '=');
    query[1].replace(query[1].begin(), query[1].end(), "%20", " ");
    res[query[0]] = query[1];
  }
  return res;
}

static std::pair<std::string, std::string> getHeaderKeyValue(const std::string &header) {
  std::string key = "", value = "";
  bool keyComplete = false;
  int i = 0, size = header.length();
  for(; i < size; i++) {
    if(header[i] == ':') {
      keyComplete = true;
    } else if(keyComplete == false)
    key += header[i];
    else if(keyComplete == true)
    value += header[i];
  }
  return {trim(key), trim(value)};
}

static Req parseHttpRequest(std::string request) {
  std::stringstream ss(request);
  std::string line;
  
  getline(ss, line);
  std::vector<std::string> firstLine = split(line, ' ');
  std::string method = firstLine[0], path = firstLine[1], protocol = firstLine[2];
  protocol.pop_back();
  int pathSize = path.length();

  std::unordered_map<std::string, std::string> headers, queryParameters;
  int queryParamPos = path.find("?");
  
  if(queryParamPos != std::string::npos)
    queryParameters = getQueryParameters(path.substr(queryParamPos, pathSize - queryParamPos - 1));
  
    while(getline(ss, line)) {
    std::pair<std::string, std::string> headerKeyValue = getHeaderKeyValue(line);
    headers.insert(headerKeyValue);
  }
  
  Req reqObj(method, path, protocol, queryParameters, headers);
  
  return reqObj;
}

class Route {
  public:
  Route() : middlewares({}), handler([](Req &req, Res &res) {}) {}
  Route(std::vector<std::function<void(Req&, Res&, std::function<void()>)>> _middlewares, std::function<void(Req&, Res&)> _handler) : middlewares(_middlewares), handler(_handler) {}
  Route(const Route &route) : middlewares(route.middlewares), handler(route.handler) {}
  std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares;
  std::function<void(Req&, Res&)> handler;
};

std::unordered_map<std::string, Route> allowed;

static void handleClientRequest(const SOCKET clientSocket) {
  char recvBuffer[1024];

  int bytesRecieved = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
  if(bytesRecieved > 0) {
    recvBuffer[bytesRecieved] = '\0';

    Req req = parseHttpRequest(std::string(recvBuffer));
    Res res;
    res.setProtocol(req.protocol);

    std::string key = req.method + "::" + req.path;

    if(allowed.find(key) == allowed.end()) {
      res.status(404)->send("Not found");
    } else {
      allowed[key].handler(req, res);
    }

    std::string httpResponse = makeHttpResponse(res);

    send(clientSocket, httpResponse.c_str(), httpResponse.length(), 0);

    std::cout << "Response sent" << std::endl;
    
  } else if(bytesRecieved == 0) {
    std::cerr << "Client disconnected" << std::endl;
    closesocket(clientSocket);
    std::cout << "Client connection closed" << std::endl;
  } else {
    std::cerr << "Error in recieving from client" << std::endl;
    closesocket(clientSocket);
    std::cout << "Client connection closed" << std::endl;
  }

  closesocket(clientSocket);
}

void serverListen(const SOCKET serverSocket) {
  while(true) {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrSize);

    if(clientSocket == INVALID_SOCKET) {
      std::cerr << "Accepting client failed" << WSAGetLastError() << std::endl;
      continue;
    }

    std::cout << "Client connected" << std::endl;

    handleClientRequest(clientSocket);
  }
}
class HttpServer {
  
  SOCKET serverSocket;
  
public:

  HttpServer() : serverSocket(INVALID_SOCKET) {}
  HttpServer(const SOCKET s) : serverSocket(s) {}


  int serverInit(int addressFamily, int type, int protocol, int port) {
    WSADATA wsadata;

    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

    if(result != 0) {
      throw std::runtime_error("DLL not found");
      return result;
    }

    SOCKET initialSocket = INVALID_SOCKET;
    initialSocket = socket(addressFamily, type, protocol);

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
      throw std::runtime_error(std::string("Error in listening") + std::to_string(WSAGetLastError()));
      return listenResult;
    }

    serverSocket = initialSocket;

    std::cout << "Server listening on port " << port << std::endl;
    std::thread connectionThread(serverListen, serverSocket);
    connectionThread.detach();

    return initialSocket;
  }

  void Get(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    std::string key = "GET::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Post(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    std::string key = "POST::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Patch(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    std::string key = "PATCH::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Put(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    std::string key = "PUT::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  void Delete(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    std::string key = "DELETE::" + path;
    allowed[key] = Route(middlewares, handler);
  }

  ~HttpServer() {
    closesocket(serverSocket);
    WSACleanup();
  }
};