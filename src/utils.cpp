#include <chrono>
#include <cstdint>
#include <random>
#include <string>

#include "utils.h"

std::string u8str2str(const std::u8string& u8str)
{
    return std::string{u8str.begin(), u8str.end()};
}

std::string path2str(const std::filesystem::path& path)
{
    return u8str2str(path.u8string());
}

int64_t get_timestamp_ns()
{
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return ns.time_since_epoch().count();
}

std::string generate_unique_key()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    int64_t timestamp_ns = get_timestamp_ns();
    uint64_t random_part = dis(gen);

    return std::to_string(timestamp_ns) + std::to_string(random_part);
}
