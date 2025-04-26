#pragma once

#include <winsock2.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <queue>
#include <condition_variable>
#include <thread>

#include "request.h"
#include "response.h"

#pragma comment(lib, "ws2_32.lib")

/**
 * @brief The HttpServer class provides a basic asynchronous HTTP server implementation using IO Completion Ports.
 *
 * This class manages incoming HTTP requests, parses them, applies global and route-specific middlewares,
 * and dispatches the request to the appropriate handler. It also creates and manages worker threads.
 */
class HttpServer {
  SOCKET serverSocket;

  /**
   * @brief The Route class represents a route with its associated middlewares and handler.
   */
  class Route {
  public:
    /**
     * @brief Default constructor for Route.
     */
    Route() : middlewares({}), handler([](Req &req, Res &res) {}) {}

    /**
     * @brief Constructs a Route with specified middlewares and a handler.
     * 
     * @param _middlewares A vector of middleware functions.
     * @param _handler A function to handle the request and response.
     */
    Route(std::vector<std::function<void(Req&, Res&, long long&)>> _middlewares,
          std::function<void(Req&, Res&)> _handler)
          : middlewares(_middlewares), handler(_handler) {}

    /**
     * @brief Copy constructor for Route.
     *
     * @param route The Route instance to copy.
     */
    Route(const Route &route) : middlewares(route.middlewares), handler(route.handler) {}

    std::vector<std::function<void(Req&, Res&, long long&)>> middlewares;  ///< Middleware functions for this route.
    std::function<void(Req&, Res&)> handler;  ///< Handler function for processing the request.
  };

  /**
   * @brief The SocketBuffer struct manages the buffer and synchronization for a given socket.
   */
  struct SocketBuffer {
    std::string buffer;   ///< Buffer to hold incoming data.
    std::mutex mtx;       ///< Mutex for synchronizing access to the buffer.
    bool processing = false;  ///< Flag to indicate if the socket is currently processing data.
  };

  struct RequestPackage {
    SOCKET socket;
    std::string rawRequest;
  };

  std::queue<RequestPackage> requestQueue;
  std::mutex queueMutex;
  std::condition_variable queueCond;

  HANDLE iocp;   ///< Handle to the IO Completion Port.
  std::vector<std::thread> workerThreads;  ///< Worker threads for asynchronous IO processing.

  // Thread-local storage for per-socket buffers.
  static thread_local std::unordered_map<SOCKET, SocketBuffer> socketBuffers;

  std::unordered_map<std::string, Route> allowedRoutes;  ///< Map storing allowed routes and their handlers.
  std::vector<std::function<void(Req&, Res&, long long&)>> globalMiddlewares;  ///< Global middleware functions.


  static const int BUFFER_SIZE = 10240;  ///< Buffer size for socket communications.
  unsigned int MAX_THREADS = 1;  ///< Maximum number of worker threads.
  size_t MAX_HEADER_SIZE = 8192;  ///< Maximum allowed header size.

  /**
   * @brief Struct to store per-IO operation data.
   */
  struct PerIoData {
    OVERLAPPED overlapped;  ///< Overlapped structure for asynchronous IO.
    WSABUF wsabuff;         ///< WSABUF structure for IO data.
    char buffer[BUFFER_SIZE];  ///< Data buffer.
    SOCKET socket;          ///< Associated socket.
    bool receiving;         ///< Flag indicating if the operation is a receive.
  };

  /**
   * @brief Gets the textual representation of an HTTP status code.
   *
   * @param statusCode The HTTP status code.
   * @return std::string The corresponding status message.
   */
  static std::string getStatusCodeWord(const int statusCode);

  /**
   * @brief Creates a full HTTP response string from a Res object.
   *
   * @param res The response object.
   * @return std::string The complete HTTP response.
   */
  static std::string makeHttpResponse(const Res &res);

  /**
   * @brief Decodes a URL-encoded special sequence into its corresponding character.
   *
   * @param specialSequence The URL-encoded sequence.
   * @return char The decoded character.
   */
  static char urlEncodingCharacter(const std::string specialSequence);

  /**
   * @brief Parses query parameters from the request URL and adds them to the Req object.
   *
   * @param req The request object.
   */
  static void parseQueryParameters(Req &req);

  /**
   * @brief Sends an error response to the client.
   *
   * @param res The response object containing the error details.
   * @param clientSocket The client socket.
   */
  static void sendErrorResponse(Res &res, const SOCKET &clientSocket);
  
  /**
   * @brief Parses an HTTP request string into a Req object.
   *
   * @param request The raw HTTP request string.
   * @param clientSocket The client socket.
   * @return Req The parsed request.
   */
  static Req parseHttpRequest(const std::string &request, const SOCKET &clientSocket);

  /**
   * @brief Function executed by worker threads to handle IO completion events.
   */
  void workerThread();

  void receiverThread();

public:
  /**
   * @brief Default constructor.
   */
  HttpServer() : serverSocket(INVALID_SOCKET) {}

  /**
   * @brief Constructs the server with a given socket.
   *
   * @param s The server socket.
   */
  HttpServer(const SOCKET s) : serverSocket(s) {}

  /**
   * @brief Sets the maximum allowed header size.
   *
   * @param maxHeaderSize Maximum header size in bytes.
   */
  inline void setMaxHeaderSize(size_t maxHeaderSize) { MAX_HEADER_SIZE = maxHeaderSize; }

  /**
   * @brief Sets the number of worker threads.
   *
   * @param threads The number of threads.
   */
  inline void setThreads(unsigned int threads) { MAX_THREADS = threads; }

  /**
   * @brief Starts listening for incoming connections on the provided server socket.
   *
   * @param serverSocket The server socket.
   */
  void serverListen();
  
  /**
   * @brief Initializes the server socket and starts the IO completion port.
   *
   * @param addressFamily The address family (e.g., AF_INET).
   * @param type The socket type (e.g., SOCK_STREAM).
   * @param protocol The protocol (e.g., IPPROTO_TCP).
   * @param port The port number.
   * @return int The socket descriptor on success.
   */
  int initServer(int addressFamily, int type, int protocol, int port);

  /**
   * @brief Adds a global middleware function that applies to all routes.
   *
   * @param middleware The middleware function.
   */
  inline void use(std::function<void(Req&, Res&, long long&)> middleware) {
    globalMiddlewares.emplace_back(middleware);
  }

  /**
   * @brief Registers a GET route with associated middlewares and a handler.
   *
   * @param path The route path.
   * @param middlewares Vector of middleware functions.
   * @param handler The handler function.
   */
  void Get(const std::string path,
           const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
           std::function<void(Req&, Res&)> handler);

  /**
   * @brief Registers a POST route with associated middlewares and a handler.
   *
   * @param path The route path.
   * @param middlewares Vector of middleware functions.
   * @param handler The handler function.
   */
  void Post(const std::string path,
            const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
            std::function<void(Req&, Res&)> handler);

  /**
   * @brief Registers a PATCH route with associated middlewares and a handler.
   *
   * @param path The route path.
   * @param middlewares Vector of middleware functions.
   * @param handler The handler function.
   */
  void Patch(const std::string path,
             const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
             std::function<void(Req&, Res&)> handler);

  /**
   * @brief Registers a PUT route with associated middlewares and a handler.
   *
   * @param path The route path.
   * @param middlewares Vector of middleware functions.
   * @param handler The handler function.
   */
  void Put(const std::string path,
           const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
           std::function<void(Req&, Res&)> handler);

  /**
   * @brief Registers a DELETE route with associated middlewares and a handler.
   *
   * @param path The route path.
   * @param middlewares Vector of middleware functions.
   * @param handler The handler function.
   */
  void Delete(const std::string path,
              const std::vector<std::function<void(Req&, Res&, long long&)>> middlewares,
              std::function<void(Req&, Res&)> handler);

  /**
   * @brief Destructor for HttpServer.
   *
   * Cleans up all resources, closes sockets, and clears data structures.
   */
  ~HttpServer() {
    workerThreads.clear();
    closesocket(serverSocket);
    for(std::pair<const SOCKET, SocketBuffer> &it : socketBuffers)
      closesocket(it.first);
    socketBuffers.clear();
    globalMiddlewares.clear();
    allowedRoutes.clear();
    WSACleanup();
  }
};
