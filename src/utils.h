#pragma once

#include <string>
#include <filesystem>

std::string u8str2str(const std::u8string& u8str);

std::string path2str(const std::filesystem::path& path);

int64_t get_timestamp_ns();

std::string generate_unique_key();
