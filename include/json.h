#pragma once

#include <variant>
#include <unordered_map>
#include <vector>
#include <string>

/**
 * @brief The JSONValue class represents a JSON value that can be of various types.
 */
class JSONValue {
public:
  using Object = std::unordered_map<std::string, JSONValue>;  ///< JSON object (dictionary).
  using Array = std::vector<JSONValue>;                         ///< JSON array (list).

  // The underlying variant that stores the JSON value.
  std::variant<std::nullptr_t, bool, double, std::string, Array, Object> value;

  /**
   * @brief Default constructor initializes the JSON value to null.
   */
  JSONValue() : value(nullptr) {}

  /**
   * @brief Constructor for null value.
   */
  JSONValue(std::nullptr_t) : value(nullptr) {}

  /**
   * @brief Constructor for boolean value.
   */
  JSONValue(bool b) : value(b) {}

  /**
   * @brief Constructor for double (number) value.
   */
  JSONValue(double d) : value(d) {}

  /**
   * @brief Constructor for string value.
   *
   * @param s The string.
   */
  JSONValue(const std::string s) : value(s) {}

  /**
   * @brief Constructor for C-string value.
   *
   * @param s The C-string.
   */
  JSONValue(const char *s) : value(std::string(s)) {}

  /**
   * @brief Constructor for array value.
   *
   * @param a The JSON array.
   */
  JSONValue(const Array a) : value(a) {}

  /**
   * @brief Constructor for object value.
   *
   * @param o The JSON object.
   */
  JSONValue(const Object o) : value(o) {}

  /**
   * @brief Copy constructor.
   *
   * @param json The JSONValue instance to copy.
   */
  JSONValue(const JSONValue &json) : value(json.value) {}

  /**
   * @brief Converts the JSON value into its string representation.
   *
   * @return std::string The JSON string.
   */
  std::string stringify() const;

  // Optional clear method (currently commented out).
  // void clear();

  // Destructor (if needed).
  // ~JSONValue() { this->clear(); }
};

/**
 * @brief The JSONParser class is responsible for parsing a JSON string into a JSONValue.
 */
class JSONParser {
  std::string input;  ///< The JSON input string.
  size_t pos, size;   ///< Current position and size of the input.

  /**
   * @brief Returns the next character and advances the position.
   *
   * @return char The next character.
   */
  inline char get() { return input[pos++]; }

  /**
   * @brief Returns the next character without advancing the position.
   *
   * @return char The next character or '\0' if end of input.
   */
  inline char peek() const { return pos < size ? input[pos] : '\0'; }

  /**
   * @brief Skips whitespace characters in the input.
   */
  void skipWhitespaces() {
    while(pos < size && std::isspace(input[pos])) {
      pos++;
    }
  }

  // Parsing helper functions:
  JSONValue parseBoolean();
  JSONValue parseNull();
  JSONValue parseString();
  JSONValue parseNumber();
  JSONValue parseObject();
  JSONValue parseArray();

public:
  /**
   * @brief Constructs a JSONParser with a given JSON string.
   *
   * @param str The JSON string to parse.
   */
  inline JSONParser(std::string &str) : input(str), pos(0), size(str.length()) {}

  /**
   * @brief Resets the parser with a new JSON string.
   *
   * @param jsonString The new JSON string.
   */
  inline void setJsonString(const std::string &jsonString) { input = jsonString; pos = 0; size = jsonString.length(); }

  /**
   * @brief Parses the JSON string and returns a JSONValue.
   *
   * @return JSONValue The parsed JSON value.
   */
  JSONValue parse();

  // Optional clear method (currently commented out).
  // inline void clear();

  // Destructor (if needed).
  // ~JSONParser() { pos = size = 0; input.~basic_string(); }
};
