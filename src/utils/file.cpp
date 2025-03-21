#include "file.h"

#include <chrono>
#include <filesystem>

namespace utils::file
{

std::tm* get_last_modified(const std::filesystem::path& path)
{
    namespace fs = std::filesystem;

    const auto file_time = std::chrono::clock_cast<std::chrono::system_clock>(fs::last_write_time(path));
    const std::time_t file_time_t = std::chrono::system_clock::to_time_t(file_time);

    return std::localtime(&file_time_t);
}

} // namespace file
