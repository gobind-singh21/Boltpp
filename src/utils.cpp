#include "utils.h"

/**
 * @brief Trims leading and trailing whitespace characters from the input string.
 *
 * @param str The string to be trimmed.
 * @return std::string The trimmed string.
 */
std::string trim(std::string_view view) {
  size_t start = 0;
  while (start < view.size() && std::isspace(view[start])) ++start;
  size_t end = view.size();
  while (end > start && std::isspace(view[end - 1])) --end;
  return std::string(view.substr(start, end - start));
}

/**
 * @brief Splits a string by a given delimiter into a vector of substrings.
 *
 * @param str The string to split.
 * @param delim The delimiter character.
 * @return std::vector<std::string> The resulting vector of substrings.
 */
std::vector<std::string> split(const std::string_view str, const char delim) {
  std::vector<std::string> res;
  size_t start = 0, end = 0;

  while (end < str.size()) {
    if (str[end] == delim) {
      if (end > start)
        res.emplace_back(str.substr(start, end - start));
      start = end + 1;
    }
    end++;
  }

  if (start < str.size()) {
    res.emplace_back(str.substr(start));
  }

  return res;
}
