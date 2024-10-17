#pragma once

#include <cstdint>
#include <filesystem>
#include <stack>

#include <pugixml.hpp>

namespace utils::webdav
{

[[nodiscard]]
pugi::xml_node generate_multistatus(pugi::xml_node& xml_doc, bool ssl_enabled, const std::string& host);

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path);

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth);

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth);

} // namespace utils::webdav
