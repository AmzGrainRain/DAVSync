#pragma once

#include <ctime>
#include <filesystem>

namespace utils::file {

std::tm* get_last_modified(const std::filesystem::path& path);

}
