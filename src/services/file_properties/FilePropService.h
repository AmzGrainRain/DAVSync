#pragma once

#include <string>
#include <unordered_set>

class FilePropService
{
public:
    virtual std::unordered_set<std::string> GetFileProp(const std::string& file_sha256) = 0;
    virtual void SetFileProp(const std::string& file_sha256, const std::string& new_prop) = 0;
};
