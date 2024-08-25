#pragma once

#include <string>

namespace utils
{

int64_t get_timestamp_ns();

std::string generate_unique_key();

std::string sha256(const std::string& text);

} // namespace utils
