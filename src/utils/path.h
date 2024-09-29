#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

namespace utils::path
{

// std::filesystem::path -> std::string
[[nodiscard, maybe_unused]]
std::string to_string(const std::filesystem::path& path) noexcept;

/*
    with_separator("\\a\\b\\c", true, '/') -> "a/b/c/"
    with_separator("\\a\\b\\c", false, '/') -> "a/b/c"
 */
[[nodiscard, maybe_unused]]
std::string with_separator(const std::filesystem::path& path, bool is_dir, char separator, uint16_t skip = 0) noexcept;

} // namespace utils::path
