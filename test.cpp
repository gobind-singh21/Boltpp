#include <iostream>
#include "json.h"

int main() {
  try {
    // Test 1: Simple object with string, number, boolean, and null
    std::cout << "Start" << std::endl;
    std::string jsonStr1 = R"({"name": "Alice", "age": 30, "isMember": true, "address": null})";
    JSONParser parser1(jsonStr1);
    JSONValue result1 = parser1.parse();
    std::cout << "Test 1 Passed: " << result1.stringify() << "\n\n";

    // Test 2: Array with mixed types
    std::string jsonStr2 = R"(["hello", 123, false, null, {"nested": "object"}])";
    JSONParser parser2(jsonStr2);
    JSONValue result2 = parser2.parse();
    std::cout << "Test 2 Passed: " << result2.stringify() << "\n\n";

    // Test 3: Nested objects and arrays
    std::string jsonStr3 = R"({
      "users": [
        {"id": 1, "name": "Bob"},
        {"id": 2, "name": "Carol"}
      ],
      "active": true
    })";
    JSONParser parser3(jsonStr3);
    JSONValue result3 = parser3.parse();
    std::cout << "Test 3 Passed: " << result3.stringify() << "\n\n";

    // Test 4: A string with escape sequences
    std::string jsonStr4 = R"("Line1\nLine2\tTabbed")";
    JSONParser parser4(jsonStr4);
    JSONValue result4 = parser4.parse();
    std::cout << "Test 4 Passed: " << result4.stringify() << "\n\n";

    // Test 5: Malformed JSON (should throw)
    std::string jsonStr5 = R"({"name": "Alice", "age": })";
    JSONParser parser5(jsonStr5);
    JSONValue result5 = parser5.parse();
    std::cout << "Test 5 Failed (this line should not be reached)\n";
  } catch(const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
  
  return 0;
}
