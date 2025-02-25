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
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")

std::string trim(std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  size_t end = str.find_last_not_of(" \t");
  if(start == std::string::npos)
    return "";
  return str.substr(start, end - start + 1);
}

std::vector<std::string> split(std::string &str, const char delim) {
  std::stringstream ss(str);
  std::string word;
  std::vector<std::string> res(0);
  while(getline(ss, word, delim))
    res.emplace_back(word);
  return res;
}

// Request object class
class Req {
public:
  
  Req() : method(""), path(""), queryParameters({}), pathParameters({}), headers({}) {}
  Req(std::string _method, std::string _path, std::unordered_map<std::string, std::string> query, std::unordered_map<std::string, std::string> params, std::unordered_map<std::string, std::string> _headers) : method(_method), queryParameters(query), pathParameters(params), headers(_headers) {}
  Req(const Req &req) {}

  std::string method;
  std::string path;
  std::unordered_map<std::string, std::string> queryParameters;
  std::unordered_map<std::string, std::string> pathParameters;
  std::unordered_map<std::string, std::string> headers;
};

// Response object class
class Res {

  int statusCode;
  std::string payload;

public:
  // Set status code of response object
  Res* status(int statusCode) {
    this->statusCode = statusCode;
    return this;
  }

  // Pass unordered map
  // and set that map as payload
  // in json format
  Res* json(std::unordered_map<std::string, std::string> map) {
    std::string res = "{";
    for(auto &it : map)
      res += "\"" + it.first + "\"" + ":" + "\"" + it.second + "\",";
    res.pop_back();
    res += '}';
    this->payload = res;
    return this;
  }

  // Pass string as data
  // and set that as a payload
  Res* send(std::string data) {
    this->payload = data;
    return this;
  }
};

class HttpServer {
  
  std::unordered_map<std::string, bool> occuredPaths;
  std::unordered_map<std::string, std::pair<std::vector<std::function<void(Req&, Res&, std::function<void()>)>>, std::function<void(Req&, Res&)>>> allowed;

  std::pair<std::string, std::string> getHeaderKeyValue(const std::string &header) {
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

  Req parseHttpRequest(std::string request) {
    std::stringstream ss(request);
    std::string line;
    getline(ss, line);
    std::vector<std::string> firstLine = split(line, ' ');
    std::string method = firstLine[0], path = firstLine[1];
    std::unordered_map<std::string, std::string> headers, queryParameters, pathParameters;
    while(getline(ss, line)) {
      std::pair<std::string, std::string> headerKeyValue = getHeaderKeyValue(line);
      headers.insert(headerKeyValue);
    }
    Req reqObj(method, path, queryParameters, pathParameters, headers);
    return reqObj;
  }

public:
  SOCKET serverSocket;

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

    pthread_t connectionThread;

    return initialSocket;
  }

  void Get(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    occuredPaths[path] = true;
    std::string key = "GET::" + path;
    allowed[key] = {middlewares, handler};
  }

  void Post(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    occuredPaths[path] = true;
    std::string key = "POST::" + path;
    allowed[key] = {middlewares, handler};
  }

  void Patch(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    occuredPaths[path] = true;
    std::string key = "PATCH::" + path;
    allowed[key] = {middlewares, handler};
  }

  void Put(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    occuredPaths[path] = true;
    std::string key = "PUT::" + path;
    allowed[key] = {middlewares, handler};
  }

  void Delete(const std::string path, const std::vector<std::function<void(Req&, Res&, std::function<void()>)>> middlewares, std::function<void(Req&, Res&)> handler) {
    occuredPaths[path] = true;
    std::string key = "DELETE::" + path;
    allowed[key] = {middlewares, handler};
  }

  ~HttpServer() {
    closesocket(serverSocket);
    WSACleanup();
  }
};