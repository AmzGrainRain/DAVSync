#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace utils::path
{

// std::filesystem::path -> std::string
[[nodiscard]]
std::string to_string(const std::filesystem::path& path);

/*
    with_separator("\\a\\b\\c", true, '/') -> "a/b/c/"
    with_separator("\\a\\b\\c", false, '/') -> "a/b/c"
 */
[[nodiscard]]
std::string with_separator(const std::filesystem::path& path, uint16_t skip, bool is_dir, char separator);

} // namespace utils::path
