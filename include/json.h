#pragma once

#include <variant>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

class JSONValue {
public:
  using Object = std::unordered_map<std::string, JSONValue>;
  using Array = std::vector<JSONValue>;

  std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value;

  JSONValue() : value(nullptr) {}
  JSONValue(std::nullptr_t) : value(nullptr) {}
  JSONValue(bool b) : value(b) {}
  JSONValue(double d) : value(d) {}
  JSONValue(const std::string s) : value(s) {}
  JSONValue(const char *s) : value(std::string(s)) {}
  JSONValue(const Array a) : value(a) {}
  JSONValue(const Object o) : value(o) {}

  std::string stringify() const;
};

class JSONParser {
  std::string input;
  size_t pos, size;

  inline char get() { return input[pos++]; }
  inline char peek() const { return pos < size ? input[pos] : '\0'; }
  void skipWhitespaces() {
    while(pos < size && std::isspace(input[pos])) {
      pos++;
    }
  }

  JSONValue parseBoolean();

  JSONValue parseNull();

  JSONValue parseString();

  JSONValue parseNumber();

  JSONValue parseObject();

  JSONValue parseArray();

public:
  inline JSONParser(std::string &str) : input(str), pos(0), size(str.length()) {}

  inline void setJsonString(const std::string &jsonString) { input = jsonString; pos = 0; size = jsonString.length(); }

  JSONValue parse();
};