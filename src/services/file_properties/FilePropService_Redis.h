#pragma once

#include "FilePropService.h"

#include <unordered_set>
#include <string>

class FilePropService_Redis: public FilePropService
{
public:
    std::unordered_set<std::string> GetFileProp(const std::string& file_sha256) override
    {
        // TODO
        return {};
    }

    void SetFileProp(const std::string& file_sha256, const std::string& new_prop) override
    {
        // TODO
    }
};
