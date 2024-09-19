#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <stack>
#include <string_view>

#include <pugixml.hpp>

namespace utils::webdav
{

pugi::xml_node generate_multistatus(pugi::xml_node& xml_doc, bool ssl_enabled, const std::string& host);

void generate_response_list(pugi::xml_node& multistatus, const std::filesystem::path& path);

void generate_response_list_recurse(pugi::xml_node& multistatus, std::stack<std::filesystem::path> dirs, int8_t depth);

void generate_response_list_recurse(pugi::xml_node& multistatus, const std::filesystem::path& path, int8_t depth);

auto uri_to_absolute(const std::filesystem::path& webdav_abslute_data_path, const std::string& webdav_prefix,
                     const std::string_view& uri) -> std::filesystem::path;

auto compute_etag(const std::filesystem::path& file_path) -> std::optional<std::string>;

} // namespace utils::webdav
