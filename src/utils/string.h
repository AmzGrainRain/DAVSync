#pragma once

#include <optional>
#include <string>
#include <vector>

namespace utils::string
{

// std::u8string -> std::string
std::string u8str2str(const std::u8string& u8str);

// replace("hello world", "hello", "perfect") -> "perfect world"
void replace(std::string& str, const std::string& search, const std::string_view& replace_with);

// trim("  hello ") -> "hello"
void trim(std::string& str);

// split("hello,world", ",") -> std::vector{"hello", "world"}
std::vector<std::string> split(const std::string& str, const std::string_view& separator);

// split2pair("name=Lee", "=") -> std::pair<std::string, std::string>("name", "Lee")
auto split2pair(const std::string& str, char separator) -> std::pair<std::string, std::string>;

auto split2pair(const std::string_view& str, char separator) -> std::pair<std::string, std::string>;

} // namespace utils::string
