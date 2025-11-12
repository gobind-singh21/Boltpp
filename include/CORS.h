#pragma once

#include <unordered_set>
#include <string>

struct CorsConfig {
  std::unordered_set<std::string> allowedOrigins;
  std::unordered_set<std::string> allowedMethods;
  std::unordered_set<std::string> allowedHeaders;
  std::unordered_set<std::string> exposedHeaders;
  bool withCredentials = false;
};