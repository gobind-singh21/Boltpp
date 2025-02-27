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

  std::string stringify() const {
    std::ostringstream oss;
    std::visit([&oss](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, std::nullptr_t>)
        oss << "null";
      else if constexpr (std::is_same_v<T, bool>)
        oss << (arg ? "true" : "false");
      else if constexpr (std::is_same_v<T, double>)
        oss << arg;
      else if constexpr (std::is_same_v<T, std::string>)
        oss << "\"" << arg << "\"";
      else if constexpr (std::is_same_v<T, Array>) {
        oss << "[";
        size_t size = arg.size();
        for(int i = 0; i < size; i++) {
          if(i > 0)
            oss << ",";
          oss << arg[i].stringify();
        }
        oss << "]";
      } else if constexpr (std::is_same_v<T, Object>) {
        oss << "{";
        bool first = true;
        for(const auto &kv : arg) {
          if(!first) oss << ",";
          first = false;
          oss << "\"" << kv.first << "\":" << kv.second.stringify();
        }
        oss << "}";
      }
    }, value);
    return oss.str();
  }
};