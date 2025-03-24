#include "utils.h"

std::string trim(const std::string str) {
  size_t start = str.find_first_not_of(" \t");
  size_t end = str.find_last_not_of(" \t");
  if(start == std::string::npos || end == std::string::npos)
    return "";
  return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string str, const char delim) {
  size_t length = str.size(), i = 0;
  std::string word;
  word.reserve(length);
  std::vector<std::string> res(0);
  while(i < length) {
    char c = str[i];
    if(c == delim) {
      res.emplace_back(word);
      word.clear();
    } else {
      word.push_back(c);
    }
  }
  if(word != "")
    res.emplace_back(word);
  return res;
}