#pragma once

#include <stdexcept>
#include <string>

class BadRequestException final : public std::runtime_error
{
  public:
    explicit BadRequestException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class ForbiddenException final : public std::runtime_error
{
  public:
    explicit ForbiddenException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class PreconditionFailedException final : public std::runtime_error
{
  public:
    explicit PreconditionFailedException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class ConflictException final : public std::runtime_error
{
  public:
    explicit ConflictException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class LockedException final : public std::runtime_error
{
  public:
    explicit LockedException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class NotFoundException final : public std::runtime_error
{
  public:
    explicit NotFoundException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};

class MethodNotAllowedException final : public std::runtime_error
{
  public:
    explicit MethodNotAllowedException(const std::string& msg) : std::runtime_error(msg)
    {
    }
};
