#pragma once

#include "json.h"

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
    if(get() != '"')
      throw std::runtime_error("Expected \" as starting of key in JSON object");
    else {
      std::string key = "";
      key.reserve(100);
      JSONValue value;
      while(true) {
        char c = get();
        if(c == ':' || pos >= size)
          throw std::runtime_error("Unterminated key");
        else if(c == '"')
          break;
        else
          key.push_back(c);
      }
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
          default: throw std::runtime_error("Unexpected symbol caught");
        }
      }

    }
    return JSONValue(obj);
  }

  JSONValue parseArray() {
    JSONValue::Array arr;
    get();
    skipWhitespaces();
    return JSONValue(arr);
  }

public:
  JSONParser(std::string &str) : input(str), pos(0), size(str.length()) {}

  JSONValue parse() {
    skipWhitespaces();
  }
};