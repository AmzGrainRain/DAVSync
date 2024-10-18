#pragma once

#include <cstdint>
#include <filesystem>
#include <stack>

#include <pugixml.hpp>

namespace utils::webdav
{

[[nodiscard]]
pugi::xml_node generate_multistatus_header(pugi::xml_node& xml_doc);

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path);

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth);

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth);

void check_precondition(const std::filesystem::path& abs_path, std::string conditions);

} // namespace utils::webdav
