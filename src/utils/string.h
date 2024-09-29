#pragma once

#include <string>
#include <vector>

namespace utils::string
{

// replace("hello world", "hello", "perfect") -> "perfect world"
void replace(std::string& str, const std::string& search, const std::string& replace_with);

// trim("  hello ") -> "hello"
void trim(std::string& str);

// split("hello,world", ",") -> std::vector{"hello", "world"}
std::vector<std::string> split(const std::string& str, const char separator);

std::vector<std::string> split(const std::string_view& str, const char separator);

// split2pair("name=Lee", "=") -> std::pair<std::string, std::string>("name", "Lee")
[[nodiscard]]
auto split2pair(const std::string& str, char separator) -> std::pair<std::string, std::string>;

[[nodiscard]]
auto split2pair(const std::string_view& str, char separator) -> std::pair<std::string, std::string>;

} // namespace utils::string
