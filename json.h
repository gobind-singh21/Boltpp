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
        output.append("null");
      else if constexpr (std::is_same_v<T, bool>)
        output.append(arg ? "true" : "false");
      else if constexpr (std::is_same_v<T, double>)
        output.append(std::to_string(arg));
      else if constexpr (std::is_same_v<T, std::string>) {
        output.append("\"");
        output.append(arg);
        output.append("\"");
      }
      else if constexpr (std::is_same_v<T, Array>) {
        output.append("[");
        size_t size = arg.size();
        for(int i = 0; i < size; i++) {
          if(i > 0)
            output.append(",");
          output.append(arg[i].stringify());
        }
        output.append("]");
      } else if constexpr (std::is_same_v<T, Object>) {
        output.append("{");
        bool first = true;
        for(const auto &kv : arg) {
          if(!first) output.append(",");
          first = false;
          output.append("\"");
          output.append(kv.first);
          output.append("\":");
          output.append(kv.second.stringify());
        }
        output.append("}");
      }
    }, value);
    return output;
  }
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

  JSONValue parseBoolean() {
    if(pos <= size - 5 && input[pos] == 'f' && input[pos + 1] == 'a' && input[pos + 2] == 'l' && input[pos + 3] == 's' && input[pos + 4] == 'e') {
      for(int i = 0; i < 5; i++)
        get();
      return JSONValue(false);
    }
    else if(pos <= size - 4 && get() == 't' && get() == 'r' && get() == 'u' && get() == 'e')
      return JSONValue(true);
    else
      throw std::runtime_error("Unexpected value caught, expected boolean");
  }

  JSONValue parseNull() {
    if(pos <= size - 4 && get() == 'n' && get() == 'u' && get() == 'l' && get() == 'l')
      return JSONValue(nullptr);
    else
      throw std::runtime_error("Unexpected value caught, expected null");
  }

  JSONValue parseString() {
    if(get() != '"')
      throw std::runtime_error("Expected '\"' at begining of the string");
    std::string output;
    output.reserve(1000);
    while(true) {
      if(pos >= size)
        throw std::runtime_error("Unterminated string");
      char c = get();
      if(c == '"')
        break;
      if(c == '\\') {
        if(pos >= size)
          throw std::runtime_error("Invalid escape sequence in string");
        char esc = get();
        switch(esc) {
          case '"': output.push_back('"'); break;
          case '\\': output.push_back('\\'); break;
          case '/': output.push_back('/'); break;
          case 'b': output.push_back('b'); break;
          case 'f': output.push_back('f'); break;
          case 'n': output.push_back('n'); break;
          case 'r': output.push_back('r'); break;
          case 't': output.push_back('t'); break;
          default:
            throw std::runtime_error("Invalid escape character in string");
        }
      } else
        output.push_back(c);
    }
    return JSONValue(output);
  }

  JSONValue parseNumber() {
    std::string number;
    number.reserve(310);
    if(peek() == '-') 
      number.push_back(get());
    while(std::isdigit(peek()))
      number.push_back(get());
    if(peek() == '.') {
      number.push_back(get());
      while(std::isdigit(peek()))
        number.push_back(get());
    }

    if(peek() == 'e' || peek() == 'E') {
      number.push_back(get());
      if(peek() == '+' || peek() == '-')
        number.push_back(get());
      while(std::isdigit(peek()))
        number.push_back(get());
    }

    double num = std::stod(number);
    return JSONValue(num);
  }

  JSONValue parseObject() {
    JSONValue::Object obj;
    get();
    skipWhitespaces();
    if(peek() == '}') {
      get();
      return JSONValue(obj);
    }
    if(peek() != '"')
      throw std::runtime_error("Expected \" as starting of key in JSON object");
    else {
      while(true) {
        JSONValue keyObj = parseString(), value;
        if(!std::holds_alternative<std::string>(keyObj.value))
          throw std::runtime_error("Object key is not a string");
        std::string key = std::get<std::string>(keyObj.value);
        skipWhitespaces();
        if(get() != ':')
          throw std::runtime_error("Missing : after key value");
        else {
          skipWhitespaces();
          char c = peek();
          switch(c) {
            case '"': value = parseString(); break;
            case '{': value = parseObject(); break;
            case '[': value = parseArray(); break;
            case 'n': value = parseNull(); break;
            case 't':
            case 'f': value = parseBoolean(); break;
            case '-':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '0': value = parseNumber(); break;
            default: throw std::runtime_error(std::string("Unexpected symbol caught: ") + c);
          }
        }
        obj[key] = value;
        skipWhitespaces();
        char c = get();
        if(c == '}')
          break;
        else if(c == ',') {
          skipWhitespaces();
          if(peek() == '}')
            throw std::runtime_error("Trailing commas not allowed in JSON object");
          if(peek() != '"')
            throw std::runtime_error("Expected \" as starting of key in JSON object");
        }
        else
          throw std::runtime_error(std::string("Expected '}' or ',' but encountered unexpected symbol: ") + c);
      }
    }
    return JSONValue(obj);
  }

  JSONValue parseArray() {
    JSONValue::Array arr;
    get();
    skipWhitespaces();
    if(peek() == ']') {
      get();
      return JSONValue(arr);
    }
    while(true) {
      char c = peek();
      JSONValue value;
      switch(c) {
        case '"': value = parseString(); break;
        case '{': value = parseObject(); break;
        case '[': value = parseArray(); break;
        case 'n': value = parseNull(); break;
        case 'f':
        case 't': value = parseBoolean(); break;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': value = parseNumber(); break;
        default: throw std::runtime_error(std::string("Unexpected symbol caught: ") + c);
      }
      skipWhitespaces();
      c = get();
      if(c == ']') {
        arr.emplace_back(value);
        break;
      }
      else if(c == ',') {
        arr.emplace_back(value);
        skipWhitespaces();
        if(peek() == ']')
          throw std::runtime_error("Trailing commas not allowed in JSON arrays");
      }
      else
        throw std::runtime_error(std::string("Unexpected symbol caught: ") + c);
    }
    return JSONValue(arr);
  }

public:
  inline JSONParser(std::string &str) : input(str), pos(0), size(str.length()) {}

  inline void setJsonString(const std::string &jsonString) { input = jsonString; }

  JSONValue parse() {
    JSONValue json;
    skipWhitespaces();
    if(pos >= size || input == "")
      return json;
    char c = peek();
    switch(c) {
      case '"': json = parseString(); break;
      case '{': json = parseObject(); break;
      case '[': json = parseArray(); break;
      case 'n': json = parseNull(); break;
      case 'f':
      case 't': json = parseBoolean(); break;
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': json = parseNumber(); break;
      default: throw std::runtime_error(std::string("Unexpected symbol caught: ") + c);
    }
    skipWhitespaces();
    if(pos >= size)
      return json;
    else
      throw std::runtime_error("Invalid JSON string value");
  }
};