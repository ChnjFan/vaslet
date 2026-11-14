// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include "app/startup_config.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <system_error>

namespace {

void ExpectDefaults(const vaslet::StartupConfig& config) {
    EXPECT_EQ(config.log.file, "vasletd.log");
    EXPECT_EQ(config.log.level, vaslet::LogLevel::kInfo);
    EXPECT_EQ(config.log.max_file_size, 0U);
    EXPECT_EQ(config.log.max_files, 0);
}

class StartupConfigTest : public testing::Test {
   protected:
    void SetUp() override {
        std::error_code error;
        const auto temporary_root = std::filesystem::temp_directory_path(error);
        ASSERT_FALSE(error) << error.message();
        const auto id =
            std::chrono::steady_clock::now().time_since_epoch().count();
        directory_ =
            temporary_root / ("vaslet-config-test-" + std::to_string(id));
        owns_directory_ = std::filesystem::create_directory(directory_, error);
        ASSERT_FALSE(error) << error.message();
        ASSERT_TRUE(owns_directory_);
        path_ = directory_ / "startup.json";
    }

    void TearDown() override {
        if (owns_directory_) {
            std::error_code error;
            std::filesystem::remove(path_, error);
            EXPECT_FALSE(error) << error.message();
            std::filesystem::remove(directory_, error);
            EXPECT_FALSE(error) << error.message();
        }
    }

    vaslet::StartupConfigResult Load(std::string_view input) const {
        std::ofstream stream(path_, std::ios::binary | std::ios::trunc);
        if (!stream.is_open()) {
            ADD_FAILURE() << "Failed to create test configuration";
            return {{}, "Test fixture failed."};
        }
        stream.write(input.data(), static_cast<std::streamsize>(input.size()));
        stream.close();
        EXPECT_FALSE(stream.fail());
        return vaslet::LoadStartupConfig(path_.string());
    }

    std::filesystem::path directory_;
    std::filesystem::path path_;

   private:
    bool owns_directory_ = false;
};

TEST_F(StartupConfigTest, UsesDefaultsForMissingFields) {
    for (const char* input : {"{}", "{\"log\":{}}"}) {
        const auto result = Load(input);
        ASSERT_TRUE(result.error.empty()) << result.error;
        ExpectDefaults(result.config);
    }
}

TEST_F(StartupConfigTest, ParsesAllLogSettings) {
    const auto result =
        Load(R"({"log":{"file":"logs/server.log","level":"debug",
        "max_file_size":1048576,"max_files":1}})");

    ASSERT_TRUE(result.error.empty()) << result.error;
    EXPECT_EQ(result.config.log.file, "logs/server.log");
    EXPECT_EQ(result.config.log.level, vaslet::LogLevel::kDebug);
    EXPECT_EQ(result.config.log.max_file_size, 1048576U);
    EXPECT_EQ(result.config.log.max_files, 1);
}

TEST_F(StartupConfigTest, AppliesDefaultsAroundAnExplicitField) {
    const auto result = Load(R"({"log":{"file":"space dir/server.log"}})");

    ASSERT_TRUE(result.error.empty()) << result.error;
    EXPECT_EQ(result.config.log.file, "space dir/server.log");
    EXPECT_EQ(result.config.log.level, vaslet::LogLevel::kInfo);
    EXPECT_EQ(result.config.log.max_file_size, 0U);
    EXPECT_EQ(result.config.log.max_files, 0);
}

TEST_F(StartupConfigTest, ParsesEverySupportedSeverity) {
    const struct {
        const char* name;
        vaslet::LogLevel level;
    } cases[] = {{"none", vaslet::LogLevel::kNone},
                 {"fatal", vaslet::LogLevel::kFatal},
                 {"error", vaslet::LogLevel::kError},
                 {"warning", vaslet::LogLevel::kWarning},
                 {"info", vaslet::LogLevel::kInfo},
                 {"debug", vaslet::LogLevel::kDebug},
                 {"verbose", vaslet::LogLevel::kVerbose}};
    for (const auto& test : cases) {
        SCOPED_TRACE(test.name);
        const auto result =
            Load(std::string("{\"log\":{\"level\":\"") + test.name + "\"}}");
        ASSERT_TRUE(result.error.empty()) << result.error;
        EXPECT_EQ(result.config.log.level, test.level);
    }
}

TEST_F(StartupConfigTest, PreservesDecodedUtf8Paths) {
    const auto result = Load(R"({"log":{"file":"unicode-\u00e9.log"}})");

    ASSERT_TRUE(result.error.empty()) << result.error;
    EXPECT_EQ(result.config.log.file, "unicode-\xC3\xA9.log");
}

TEST_F(StartupConfigTest, ReadsAcrossBufferBoundaries) {
    const auto result = Load(std::string(5000, ' ') + "{}");

    ASSERT_TRUE(result.error.empty()) << result.error;
    ExpectDefaults(result.config);
}

TEST_F(StartupConfigTest, RejectsMalformedJson) {
    for (const char* input :
         {"", " ", "{", "{\"log\":}", "{} {}", "{\"log\":{},}",
          "{/* comment */}", "{\"log\":{\"max_files\":NaN}}"}) {
        SCOPED_TRACE(input);
        const auto result = Load(input);
        EXPECT_FALSE(result.error.empty());
        ExpectDefaults(result.config);
    }
}

TEST_F(StartupConfigTest, RejectsRawNulAndInvalidUtf8) {
    for (const std::string& input :
         {std::string("{}\0extra", 8),
          std::string("{\"log\":{\"file\":\"") + '\xFF' + "\"}}"}) {
        const auto result = Load(input);
        EXPECT_FALSE(result.error.empty());
        ExpectDefaults(result.config);
    }
}

TEST_F(StartupConfigTest, RejectsInvalidFieldsWithoutImplicitConversions) {
    const char* cases[] = {
        "[]",
        "null",
        "true",
        "1",
        "\"config\"",
        R"({"other":1})",
        R"({"log":null})",
        R"({"log":[]})",
        R"({"log":{"other":1}})",
        R"({"log":{},"log":{}})",
        R"({"log":{"file":"a","file":"b"}})",
        R"({"log":{"file":"a","fi\u006ce":"b"}})",
        R"({"log":{"level":"info","level":"debug"}})",
        R"({"log":{"max_file_size":0,"max_file_size":1000}})",
        R"({"log":{"max_files":0,"max_files":1}})",
        R"({"log":{"file":""}})",
        R"({"log":{"file":null}})",
        R"({"log":{"file":1}})",
        R"({"log":{"file":"a\u0000b"}})",
        R"({"log":{"level":"INFO"}})",
        R"({"log":{"level":"info-extra"}})",
        R"({"log":{"level":""}})",
        R"({"log":{"level":false}})",
        R"({"log":{"level":"info\u0000"}})",
        R"({"log":{"max_file_size":-1}})",
        R"({"log":{"max_file_size":999}})",
        R"({"log":{"max_file_size":1000.0}})",
        R"({"log":{"max_file_size":"1000"}})",
        R"({"log":{"max_file_size":18446744073709551616}})",
        R"({"log":{"max_files":-1}})",
        R"({"log":{"max_files":1.5}})",
        R"({"log":{"max_files":"1"}})",
        R"({"log":{"max_files":null}})",
        R"({"log":{"max_files":true}})",
        R"({"log":{"max_files":2147483648}})"};
    for (const char* input : cases) {
        SCOPED_TRACE(input);
        const auto result = Load(input);
        EXPECT_FALSE(result.error.empty());
        ExpectDefaults(result.config);
    }
}

TEST_F(StartupConfigTest, AcceptsRotationBoundaryValues) {
    const auto result = Load(
        "{\"log\":{\"max_file_size\":" +
        std::to_string(std::numeric_limits<std::size_t>::max()) +
        ",\"max_files\":" + std::to_string(std::numeric_limits<int>::max()) +
        "}}");

    ASSERT_TRUE(result.error.empty()) << result.error;
    EXPECT_EQ(result.config.log.max_file_size,
              std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(result.config.log.max_files, std::numeric_limits<int>::max());
    EXPECT_TRUE(
        Load(R"({"log":{"max_file_size":1000,"max_files":1}})").error.empty());
    EXPECT_TRUE(
        Load(R"({"log":{"max_file_size":0,"max_files":2}})").error.empty());
    EXPECT_TRUE(
        Load(R"({"log":{"max_file_size":1000,"max_files":0}})").error.empty());
}

TEST_F(StartupConfigTest, ReportsFileAccessFailures) {
    EXPECT_FALSE(vaslet::LoadStartupConfig(path_.string()).error.empty());
    const auto directory = vaslet::LoadStartupConfig(directory_.string());
    EXPECT_NE(directory.error.find("Failed to"), std::string::npos);
    EXPECT_FALSE(vaslet::LoadStartupConfig("").error.empty());
    EXPECT_FALSE(vaslet::LoadStartupConfig(std::string("invalid\0path", 12))
                     .error.empty());
}

TEST_F(StartupConfigTest,
       FailureReturnsDefaultsWithoutLeakingInputOrChangingFile) {
    const std::string input =
        R"({"log":{"file":"secret_marker","level":"invalid_secret"}})";
    const auto result = Load(input);

    ASSERT_FALSE(result.error.empty());
    ExpectDefaults(result.config);
    EXPECT_EQ(result.error.find("secret"), std::string::npos);
    std::ifstream stream(path_, std::ios::binary);
    const std::string unchanged{std::istreambuf_iterator<char>(stream),
                                std::istreambuf_iterator<char>()};
    EXPECT_EQ(unchanged, input);
}

}  // namespace
