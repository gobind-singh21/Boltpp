#include <stdexcept>
#include <charconv>
#include "json.h"

void JSONValue::stringifyTo(std::string &out) const {
  std::visit([&out](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, std::nullptr_t>)
      out.append("null");
    else if constexpr (std::is_same_v<T, bool>)
      out.append(arg ? "true" : "false");
    else if constexpr (std::is_same_v<T, double>)
      out.append(std::to_string(arg));
    else if constexpr (std::is_same_v<T, std::string>) {
      out.push_back('"');
      out.append(arg);
      out.push_back('"');
    } else if constexpr (std::is_same_v<T, Array>) {
      out.push_back('[');
      for (size_t i = 0; i < arg.size(); ++i) {
        if (i > 0) out.push_back(',');
        arg[i].stringifyTo(out);
      }
      out.push_back(']');
    } else if constexpr (std::is_same_v<T, Object>) {
      out.push_back('{');
      bool first = true;
      for (const auto& kv : arg) {
        if (!first) out.push_back(',');
        first = false;
        out.push_back('"');
        out.append(kv.first);
        out.append("\":");
        kv.second.stringifyTo(out);
      }
      out.push_back('}');
    }
  }, value);
}

JSONValue& JSONValue::operator=(const JSONValue json) {
  value = json.value;
  return *this;
}

JSONValue& JSONValue::operator=(const JSONValue::Object object) {
  value = object;
  return *this;
}

JSONValue& JSONValue::operator=(const JSONValue::Array array) {
  value = array;
  return *this;
}

JSONValue& JSONValue::operator=(const double number) {
  value = number;
  return *this;
}

JSONValue& JSONValue::operator=(const char* str) {
  value = std::string(str);
  return *this;
}

JSONValue& JSONValue::operator=(const std::string str) {
  value = str;
  return *this;
}

JSONValue& JSONValue::operator=(const bool boolean) {
  value = boolean;
  return *this;
}

JSONValue& JSONValue::operator=(const std::nullptr_t null) {
  value = null;
  return *this;
}

JSONValue& JSONValue::operator[](const char* str) {
  std::string key(str);
  if(JSONValue::Object* object = std::get_if<JSONValue::Object>(&this->value)) {
    auto it = object->find(key);
    if(it == object->end()) {
      object->insert({key, JSONValue(nullptr)});
    }
    it = object->find(key);
    return it->second;
  } else {
    throw std::runtime_error("Invalid [std::string] operator on a non object value");
  }
}

JSONValue& JSONValue::operator[](const std::string key) {
  if(JSONValue::Object* object = std::get_if<JSONValue::Object>(&this->value)) {
    auto [it, _] = object->try_emplace(key, nullptr);
    return it->second;
  } else {
    throw std::runtime_error("Invalid [std::string] operator on a non object value");
  }
}

JSONValue& JSONValue::operator[](const int index) {
  if(JSONValue::Array* array = std::get_if<JSONValue::Array>(&this->value)) {
    int length = array->size();
    if(index >= length) {
      throw std::runtime_error("Out of bounds index for array value");
    }
    return (*array)[index];
  } else {
    throw std::runtime_error("Used [int] operator on a non array value");
  }
}

double& JSONValue::asDouble() {
  if(double* number = std::get_if<double>(&this->value)) {
    return *number;
  } else {
    throw std::runtime_error("asDouble() used on a non double type JSONValue");
  }
}

std::string& JSONValue::asString() {
  if(std::string* str = std::get_if<std::string>(&this->value)) {
    return *str;
  } else {
    throw std::runtime_error("asString() used on a non std::string type JSONValue");
  }
}

bool& JSONValue::asBool() {
  if(bool* boolean = std::get_if<bool>(&this->value)) {
    return *boolean;
  } else {
    throw std::runtime_error("asBool() used on a non boolean type JSONValue");
  }
}

std::nullptr_t& JSONValue::asNull() {
  if(std::nullptr_t* null = std::get_if<std::nullptr_t>(&this->value)) {
    return *null;
  } else {
    throw std::runtime_error("asNull() used on a non null type JSONValue");
  }
}

/**
 * @brief Converts the JSONValue into a JSON-formatted string.
 *
 * Uses std::visit to handle all variant types.
 *
 * @return std::string The JSON string.
 */
std::string JSONValue::stringify() const {
  std::string output = "";
  output.reserve(2048);
  stringifyTo(output);
  return output;
}

/**
 * @brief Parses a JSON boolean value from the input.
 *
 * @return JSONValue The parsed boolean.
 */
JSONValue JSONParser::parseBoolean() {
  if (input.compare(pos, 4, "true") == 0) {
    pos += 4;
    return JSONValue(true);
  }
  if (input.compare(pos, 5, "false") == 0) {
    pos += 5;
    return JSONValue(false);
  }
  else
    throw std::runtime_error("Unexpected value caught, expected boolean");
}

/**
 * @brief Parses a JSON null value.
 *
 * @return JSONValue The parsed null value.
 */
JSONValue JSONParser::parseNull() {
  if(pos <= size - 4 && get() == 'n' && get() == 'u' && get() == 'l' && get() == 'l')
    return JSONValue(nullptr);
  else
    throw std::runtime_error("Unexpected value caught, expected 'null'");
}

/**
 * @brief Parses a JSON string value.
 *
 * @return JSONValue The parsed string.
 */
JSONValue JSONParser::parseString() {
  if(get() != '"')
    throw std::runtime_error("Expected '\"' at beginning of the string");
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

/**
 * @brief Parses a JSON number.
 *
 * @return JSONValue The parsed number.
 */
JSONValue JSONParser::parseNumber() {
  const char* start = &input[pos];
  while (pos < size && (std::isdigit(input[pos]) || input[pos] == '-' || input[pos] == '+' || input[pos] == '.' || input[pos] == 'e' || input[pos] == 'E')) {
    pos++;
  }
  std::string_view numberView(start, &input[pos] - start);
  double num;
  auto res = std::from_chars(numberView.data(), numberView.data() + numberView.size(), num);
  if (res.ec != std::errc()) throw std::runtime_error("Invalid number");
  return JSONValue(num);
}

/**
 * @brief Parses a JSON object.
 *
 * @return JSONValue The parsed object.
 */
JSONValue JSONParser::parseObject() {
  JSONValue::Object obj;
  obj.reserve(128);
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

/**
 * @brief Parses a JSON array.
 *
 * @return JSONValue The parsed array.
 */
JSONValue JSONParser::parseArray() {
  JSONValue::Array arr;
  arr.reserve(128);
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

/**
 * @brief Parses the entire JSON input string.
 *
 * @return JSONValue The resulting JSON value.
 */
JSONValue JSONParser::parse() {
  skipWhitespaces();
  if(pos >= size || input == "")
    return JSONValue();
  JSONValue json;
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