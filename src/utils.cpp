#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <string_view>
#include <vector>

#include <PicoSHA2/picosha2.h>

#include "utils.h"

namespace utils
{

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

std::string sha256(const std::string& text)
{
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(text.begin(), text.end(), hash.begin(), hash.end());

    return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
}

} // namespace utils
