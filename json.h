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
    std::string output = "";
    std::visit([&output](auto&& arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, std::nullptr_t>)
        output += "null";
      else if constexpr (std::is_same_v<T, bool>)
        output += (arg ? "true" : "false");
      else if constexpr (std::is_same_v<T, double>)
        output += std::to_string(arg);
      else if constexpr (std::is_same_v<T, std::string>)
        output += "\"" + arg + "\"";
      else if constexpr (std::is_same_v<T, Array>) {
        output += "[";
        size_t size = arg.size();
        for(int i = 0; i < size; i++) {
          if(i > 0)
            output += ",";
          output += arg[i].stringify();
        }
        output += "]";
      } else if constexpr (std::is_same_v<T, Object>) {
        output += "{";
        bool first = true;
        for(const auto &kv : arg) {
          if(!first) output += ",";
          first = false;
          output += "\"" + kv.first + "\":" + kv.second.stringify();
        }
        output += "}";
      }
    }, value);
    return output;
  }
};