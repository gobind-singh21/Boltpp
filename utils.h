#pragma once

#include <vector>
#include <sstream>

std::string trim(const std::string &str) {
  size_t start = str.find_first_not_of(" \t");
  size_t end = str.find_last_not_of(" \t");
  if(start == std::string::npos || end == std::string::npos)
    return "";
  return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string &str, const char delim) {
  std::stringstream ss(str);
  std::string word;
  std::vector<std::string> res(0);
  while(getline(ss, word, delim)) {
    if(word == "")
    continue;
    res.emplace_back(word);
  }
  return res;
}