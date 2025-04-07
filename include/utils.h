#pragma once

#include <vector>
#include <string>

/**
 * @brief Trims whitespace from both ends of the input string.
 *
 * @param str The string to trim.
 * @return std::string The trimmed string.
 */
std::string trim(const std::string str);

/**
 * @brief Splits a string into a vector of substrings based on a delimiter.
 *
 * @param str The string to split.
 * @param delim The delimiter character.
 * @return std::vector<std::string> A vector of substrings.
 */
std::vector<std::string> split(const std::string str, const char delim);
