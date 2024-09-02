#pragma once

#include <cstdint>
#include <ctime>
#include <filesystem>
#include <optional>
#include <stack>
#include <string_view>

#include <pugixml.hpp>

namespace utils::webdav
{

pugi::xml_node generate_multistatus(pugi::xml_node& xml_doc);

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path);

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth);

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth);

std::filesystem::path uri_to_absolute(const std::string_view& uri);

std::optional<std::string> compute_etag(const std::filesystem::path& file_path);

} // namespace utils::webdav
