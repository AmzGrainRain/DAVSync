#pragma once

#include <cstdint>
#include <filesystem>
#include <stack>

#include <pugixml.hpp>
#include <string_view>

namespace utils::webdav
{

[[nodiscard]]
pugi::xml_node generate_multistatus(pugi::xml_node& xml_doc, bool ssl_enabled, const std::string& host);

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path);

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth);

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth);

// "s25axvg3" -> "urn:sha256:s25axvg3"
std::string lock_token_to_urn(const std::string& token);

// "urn:sha256:s25axvg3" -> "s25axvg3"
std::string urn_to_lock_token(const std::string& urn_str);

// "Second-1246" -> 1246
// "Infinite" -> The maximum value of long
// All other situations are considered as er rors and return the minimum value of long long
long long parse_timeout_header(const std::string_view& timeout_header_str);

} // namespace utils::webdav
