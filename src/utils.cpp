#include <chrono>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include <PicoSHA2/picosha2.h>

#include "utils.h"

constexpr auto MAP = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789+/";

constexpr auto MAP_URL_ENCODED = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789-_";

std::vector<int> create_reverse_map(const char* map)
{
    std::vector<int> reverse_map(256, -1);
    for (int i = 0; i < 64; ++i)
    {
        reverse_map[static_cast<unsigned char>(map[i])] = i;
    }
    return reverse_map;
}

const std::vector<int> REVERSE_MAP = create_reverse_map(MAP);
const std::vector<int> REVERSE_MAP_URL_ENCODED = create_reverse_map(MAP_URL_ENCODED);

namespace utils
{

std::string generate_unique_key()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    int64_t timestamp_ns = get_timestamp<std::chrono::nanoseconds>().count();
    uint64_t random_part = dis(gen);

    return std::to_string(timestamp_ns) + std::to_string(random_part);
}

std::string sha256(const std::string& text)
{
    std::string sha{};
    picosha2::hash256_hex_string(text, sha);

    return sha;
}

std::string sha256(const std::filesystem::path& file)
{
    std::ifstream f(file, std::ios::binary);
    std::vector<unsigned char> s(picosha2::k_digest_size);
    picosha2::hash256(f, s.begin(), s.end());

    return picosha2::bytes_to_hex_string(s.begin(), s.end());
}

std::string base64_decode(const std::string& src, bool url_encoded)
{
    const std::vector<int>& reverse_map = url_encoded ? REVERSE_MAP_URL_ENCODED : REVERSE_MAP;
    std::string decoded;
    decoded.reserve(src.size() * 3 / 4);

    size_t padding = 0;
    {
        auto it = src.end();
        while (it != src.begin() && (*--it == '=' || *it == '\0'))
        {
            ++padding;
        }
    }
    if (padding > 2)
    {
        throw std::invalid_argument("Invalid Base64 encoding");
    }

    auto process_quad = [&](char b1, char b2, char b3, char b4) {
        int v1 = reverse_map[static_cast<unsigned char>(b1)];
        int v2 = reverse_map[static_cast<unsigned char>(b2)];
        int v3 = reverse_map[static_cast<unsigned char>(b3)];
        int v4 = reverse_map[static_cast<unsigned char>(b4)];
        if (v1 == -1 || v2 == -1 || v3 == -1 || v4 == -1)
        {
            throw std::invalid_argument("Invalid Base64 encoding");
        }
        decoded.push_back((v1 << 2) | (v2 >> 4));
        if (v3 != 64)
        {
            decoded.push_back((v2 << 4) | (v3 >> 2));
        }
        if (v4 != 64)
        {
            decoded.push_back((v3 << 6) | v4);
        }
    };

    for (auto it = src.begin(); it + 3 < src.end(); it += 4)
    {
        process_quad(*it, *(it + 1), *(it + 2), *(it + 3));
    }

    if (src.size() % 4 != 0)
    {
        throw std::invalid_argument("Invalid Base64 encoding");
    }

    switch (src.size() % 4)
    {
    case 2:
        process_quad(src[src.size() - 2], src[src.size() - 1], '=', '=');
        break;
    case 3:
        process_quad(src[src.size() - 3], src[src.size() - 2], src[src.size() - 1], '=');
        break;
    default:
        break;
    }

    return decoded;
}

} // namespace utils
