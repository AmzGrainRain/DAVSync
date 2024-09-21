#pragma once

#include <filesystem>
#include <string>

namespace utils
{

int64_t get_timestamp_ns();

std::string generate_unique_key();

std::string sha256(const std::string& text);

std::string sha256(const std::filesystem::path& file);

} // namespace utils
