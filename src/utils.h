#pragma once

#include <chrono>
#include <filesystem>
#include <string>

namespace utils
{

std::string generate_unique_key();

std::string sha256(const std::string& text);

std::string sha256(const std::filesystem::path& file);

std::string base64_decode(const std::string &src, bool url_encoded = false);

template <class T>
concept IsTimeType = std::is_same_v<T, std::chrono::nanoseconds> || std::is_same_v<T, std::chrono::microseconds> ||
                     std::is_same_v<T, std::chrono::milliseconds> || std::is_same_v<T, std::chrono::seconds>;

template <IsTimeType T = std::chrono::nanoseconds> T get_timestamp()
{
    using namespace std::chrono;
    return duration_cast<T>(system_clock::now().time_since_epoch());
}

} // namespace utils
