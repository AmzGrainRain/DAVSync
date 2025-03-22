#include "lock.h"

#include <cstddef>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <pugixml.hpp>

#include "ConfigManager.h"
#include "http_exceptions.hpp"
#include "logger.hpp"
#include "services/FileLockService.h"
#include "utils/webdav.h"

/*

If = "If" ":" ( 1*No-tag-list | 1*Tagged-list )

No-tag-list = List
Tagged-list = Resource-Tag 1*List

List = "(" 1*Condition ")"
Condition = ["Not"] (State-token | "[" entity-tag "]")
; entity-tag: 被锁定的资源的路径的 SHA256 值，其中不能包含 ' '、'\t'、'\r'、'\n' 字符。
; 形如 [a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e]

State-token = "<" absolute-URI ">"
; absolute-URI 是锁令牌。
; 形如 <urn:uuid:58f202ac-22cf-11d1-b12d-002035b29092>

Resource-Tag = "<" Simple-ref ">"
; Simple-ref 锁定的资源的 URL, 不包含协议头、主机、端口部分，不能包含 ' '、'\t'、'\r'、'\n' 字符。
; 形如 </path/to/locked/resource>

(<lock-token>)
([entity-tag])
(<lock-token> [entity-tag])
"Not" can be placed before "<lock-token>" or "[entity-tag]"
"<Simple-ref>" can be placed before all of conditions

正则匹配：
absolute-URI = ^<urn:uuid:(.*?)>$
entity-tag = ^\[[A-Za-z0-9]{64}\]$
Simple-ref = ^\<(\/[^ \t\r\n]*\/?)>$
*/

/*
TimeOut = "Timeout" ":" 1#TimeType
TimeType = ("Second-" DAVTimeOutVal | "Infinite")
           ; No LWS allowed within TimeType
DAVTimeOutVal = 1*DIGIT

Timeout: Second-digits
Timeout: Infinite

正则匹配：
Timeout = ^Timeout: (Second-(\d+)|Infinite)$
 */

inline static void EnsurePathExists(const std::filesystem::path& path)
{
    if (std::filesystem::exists(path))
    {
        return;
    }

    if (std::filesystem::is_directory(path))
    {
        std::filesystem::create_directories(path);
    }
    else if (std::filesystem::is_regular_file(path))
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs{path};
        if (!ofs.is_open())
        {
            return;
        }
    }

    throw std::filesystem::filesystem_error("ensure path exists failed", path, std::error_code{});
}

namespace Routes::WebDAV
{

// https://fullstackplayer.github.io/WebDAV-RFC4918-CN/06-%E9%94%81%E5%AE%9A.html
void LOCK(cinatra::coro_http_request& req, cinatra::coro_http_response& res)
{
    namespace fs = std::filesystem;

    try
    {
        static const auto& conf = ConfigManager::GetInstance();
        static auto& lock_service = FileLock::Service::GetInstance();
        const std::string& username = req.get_aspect_data()[0];

        fs::path abs_path = conf.GetWebDavAbsoluteDataPath(req.get_url());
        if (abs_path.empty())
        {
            throw ForbiddenException("path not found");
        }

        // check if the resource is locked by an exclusive lock
        if (lock_service.HoldingExclusiveLock(abs_path))
        {
            throw ConflictException(std::format("exclusive lock held by {}", username));
        }

        // if header
        std::string_view if_header_value{req.get_header_value("If")};
        if (if_header_value.empty())
        {
            throw BadRequestException("if header missing");
        }
        utils::webdav::check_precondition(abs_path, std::string{if_header_value});

        // lock depth
        short lock_depth = std::numeric_limits<short>::max();
        {
            std::string depth_header{req.get_header_value("Depth")};
            if (depth_header.empty())
            {
                throw BadRequestException("depth header missing");
            }
            lock_depth = depth_header == "Infinity" ? std::numeric_limits<short>::max() : std::stoi(depth_header);
        }

        // lock expires time
        long long lock_expires = std::numeric_limits<long long>::min();
        {
            std::string timeout_header{req.get_header_value("Timeout")};
            if (timeout_header.empty())
            {
                throw BadRequestException("timeout header missing");
            }

            if (timeout_header.starts_with("Second-"))
            {
                lock_expires = std::stoll(timeout_header.substr(7));
            }
            else if (timeout_header == "Infinite")
            {
                lock_expires = std::numeric_limits<long long>::max();
            }
            else
            {
                throw BadRequestException("invalid timeout header");
            }
        }

        // parse xml body
        std::string lock_description = std::format("owner: {}, ", username);
        FileLock::LockScope lock_scope;
        FileLock::LockType lock_type;
        {
            pugi::xml_document doc;
            std::string body_str{req.get_body()};
            if (!doc.load_string(body_str.c_str()))
            {
                throw std::runtime_error("parse xml body failed");
            }

            // lock scope
            auto is_shared = doc.select_node("/D:lockinfo/D:lockscope/D:shared");
            auto is_exclusive = doc.select_node("/D:lockinfo/D:lockscope/D:exclusive");
            if (is_shared && !is_exclusive)
                lock_scope = FileLock::LockScope::SHARED;
            else if (!is_shared && is_exclusive)
                lock_scope = FileLock::LockScope::EXCLUSIVE;
            else
            {
                throw BadRequestException("lock scope missing");
            }

            // lock type
            auto is_read = doc.select_node("/D:lockinfo/D:locktype/D:read");
            auto is_write = doc.select_node("/D:lockinfo/D:locktype/D:write");
            if (is_read && !is_write)
                lock_type = FileLock::LockType::READ;
            else if (!is_read && is_write)
                lock_type = FileLock::LockType::WRITE;
            else
            {
                throw BadRequestException("lock type missing");
            }

            // lock description
            auto desc = doc.select_node("/D:lockinfo/D:owner");
            if (desc)
            {
                for (auto n : desc.node().children())
                {
                    lock_description += n.name();
                    lock_description += ':';
                    lock_description += n.text().as_string();
                    lock_description += ',';
                }
                size_t end = lock_description.length() - 1;
                lock_description = lock_description.substr(0, std::max(static_cast<size_t>(0), end));
            }
        }

        // if the URL is not mapped to any resource during the request, a new empty resource will be created and directly locked.
        EnsurePathExists(abs_path);

        // create a lock
        FileLock::EntryLock lock;
        lock.user = std::move(username);
        lock.token = std::format("urn:uuid:{}", boost::uuids::to_string(boost::uuids::random_generator()()));
        lock.description = std::move(lock_description);
        lock.depth = lock_depth;
        lock.scope = lock_scope;
        lock.type = lock_type;
        lock.expires_at = lock_expires;

        // set response header
        res.add_header("Lock-Token", lock.token);

        // xml response body
        pugi::xml_document resp_xml_body;
        {
            auto root_node = resp_xml_body.append_child("D:prop");
            root_node.append_attribute("xmlns:D").set_value("DAV:");
            root_node = root_node.append_child("D:lockdiscovery");
            root_node = root_node.append_child("D:activelock");
            root_node.append_child("D:lockscope").append_child(lock_scope == FileLock::LockScope::SHARED ? "D:shared" : "D:exclusive");
            root_node.append_child("D:locktype").append_child(lock_type == FileLock::LockType::READ ? "D:read" : "D:write");
            root_node.append_child("D:depth").set_value(lock_depth == std::numeric_limits<short>::max() ? "Infinity"
                                                                                                        : std::to_string(lock.depth).c_str());
            root_node.append_child("D:owner").append_child("D:href").set_value(lock.user.c_str());
            root_node.append_child("D:timeout").set_value(std::format("Second-{}", lock.expires_at).c_str());
            root_node.append_child("D:locktoken").append_child("D:href").set_value(lock.token.c_str());
            root_node.append_child("D:lockroot").append_child("D:href").set_value(abs_path.string().c_str());
        }

        // lock the resource
        if (!lock_service.Lock(abs_path, std::move(lock)))
        {
            throw std::runtime_error("locking failed");
        }

        // response
        std::stringstream resp_body_stream;
        resp_xml_body.save(resp_body_stream);
        res.set_content_type<cinatra::resp_content_type::xml>();
        res.set_content(resp_body_stream.str());
    }
    catch (const ForbiddenException& err)
    {
        LOG_WARN(err.what())
        res.set_status_and_content_view(cinatra::status_type::forbidden, err.what());
    }
    catch (const ConflictException& err)
    {
        LOG_WARN(err.what())
        res.set_status_and_content_view(cinatra::status_type::conflict, err.what());
    }
    catch (const BadRequestException& err)
    {
        LOG_WARN(err.what())
        res.set_status_and_content_view(cinatra::status_type::bad_request, err.what());
    }
    catch (const std::exception& err)
    {
        LOG_ERROR(err.what())
        res.set_status_and_content(cinatra::status_type::internal_server_error, err.what());
    }
}

} // namespace Routes::WebDAV
