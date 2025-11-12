#pragma once

#include <stdexcept>

class json_type_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class json_parse_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class file_not_found : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};