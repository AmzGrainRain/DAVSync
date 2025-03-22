#include "ConfigManager.h"
#include <filesystem>
#include <gtest/gtest.h>
#include <type_traits>

template <class T>
using remove_rc_t = std::remove_const_t<std::remove_reference_t<T>>;

TEST(TestConfigManager, SaveConfig)
{
    ConfigManager instance{"./config.json"};
    instance.SaveConfig();
}

TEST(TestConfigManager, LoadConfig)
{
    ConfigManager instance{"./config.json"};
    instance.LoadConfig(std::filesystem::current_path() / "./config.json");
}

TEST(TestConfigManager, ReloadConfig)
{
    ConfigManager instance{"./config.json"};
    instance.ReloadConfig();
}

TEST(TestConfigManager, GetGlobalConfig)
{
    ConfigManager instance{"./config.json"};
    const Config& config = instance.GetGlobalConfig();

    ::testing::StaticAssertTypeEq<HttpConfig, remove_rc_t<decltype(config.http)>>();
    ::testing::StaticAssertTypeEq<HttpsConfig, remove_rc_t<decltype(config.https)>>();
    ::testing::StaticAssertTypeEq<WebDavConfig, remove_rc_t<decltype(config.webdav)>>();
    ::testing::StaticAssertTypeEq<RedisConfig, remove_rc_t<decltype(config.redis)>>();
    ::testing::StaticAssertTypeEq<EngineConfig, remove_rc_t<decltype(config.engine)>>();
    ::testing::StaticAssertTypeEq<DataConfig, remove_rc_t<decltype(config.data)>>();
}

TEST(TestConfigManager, GetHttpConfig)
{
    ConfigManager instance{"./config.json"};
    const HttpConfig& http_config = instance.GetHttpConfig();

    EXPECT_EQ(http_config.host, "127.0.0.1");
    EXPECT_EQ(http_config.address, "0.0.0.0");
    EXPECT_EQ(http_config.port, 8110);
    EXPECT_EQ(http_config.buffer_size, 1024);
    EXPECT_EQ(http_config.max_thread, 0);
}

TEST(TestConfigManager, GetHttpsConfig)
{
    ConfigManager instance{"./config.json"};
    const HttpsConfig& https_config = instance.GetHttpsConfig();

    EXPECT_EQ(https_config.enable, false);
    EXPECT_EQ(https_config.port, 8111);
    EXPECT_EQ(https_config.https_only, false);
    EXPECT_EQ(https_config.cert, "fullchain.pem");
    EXPECT_EQ(https_config.key, "privkey.pem");
}

TEST(TestConfigManager, GetWebDavConfig)
{
    ConfigManager instance{"./config.json"};
    const WebDavConfig& webdav_config = instance.GetWebDavConfig();

    EXPECT_EQ(webdav_config.prefix, "/webdav");
    EXPECT_EQ(webdav_config.data_path, "./data");
    EXPECT_EQ(webdav_config.max_recurse_depth, 4);
    EXPECT_EQ(webdav_config.realm, "WEBDAV_REALM");
    EXPECT_EQ(webdav_config.verification, "basic");
    EXPECT_EQ(webdav_config.users.size(), 1);
    EXPECT_EQ(webdav_config.users[0].name, "test");
    EXPECT_EQ(webdav_config.users[0].password, "passw0rd");
}

TEST(TestConfigManager, GetRedisConfig)
{
    ConfigManager instance{"./config.json"};
    const RedisConfig& redis_config = instance.GetRedisConfig();

    EXPECT_EQ(redis_config.host, "127.0.0.1");
    EXPECT_EQ(redis_config.port, 6379);
    EXPECT_EQ(redis_config.username, "");
    EXPECT_EQ(redis_config.password, "");
}

TEST(TestConfigManager, GetEngineConfig)
{
    ConfigManager instance{"./config.json"};
    const EngineConfig& engine_config = instance.GetEngineConfig();

    EXPECT_EQ(engine_config.etag, "sqlite");
    EXPECT_EQ(engine_config.prop, "sqlite");
}

TEST(TestConfigManager, GetDataConfig)
{
    ConfigManager instance{"./config.json"};
    const DataConfig& data_config = instance.GetDataConfig();

    EXPECT_EQ(data_config.sqlite_db, "./metadata/data.db");
    EXPECT_EQ(data_config.etag_data, "./metadata/etag.db");
    EXPECT_EQ(data_config.prop_data, "./metadata/prop.db");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
