#pragma once

#include <stdexcept>
#include <string>

class BadRequestException : public std::runtime_error
{
  public:
    explicit BadRequestException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class ForbiddenException : public std::runtime_error
{
  public:
    explicit ForbiddenException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class PreconditionFailedException : public std::runtime_error
{
  public:
    explicit PreconditionFailedException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class ConflictException : public std::runtime_error
{
  public:
    explicit ConflictException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};
