// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <regex>
#include <string>
#include <system_error>

#include "common/log.h"

namespace {

class PlogTest : public testing::Test {
   protected:
    void SetUp() override {
        std::error_code error;
        const auto temporary_root = std::filesystem::temp_directory_path(error);
        ASSERT_FALSE(error) << error.message();
        const auto id =
            std::chrono::steady_clock::now().time_since_epoch().count();
        directory_ =
            temporary_root / ("vaslet-plog-test-" + std::to_string(id));
        owns_directory_ = std::filesystem::create_directory(directory_, error);
        ASSERT_FALSE(error) << error.message();
        ASSERT_TRUE(owns_directory_);
        path_ = (directory_ / "test.log").string();
    }

    void TearDown() override {
        if (owns_directory_) {
            std::error_code error;
            std::filesystem::remove(directory_ / "test.log", error);
            EXPECT_FALSE(error) << error.message();
            std::filesystem::remove(directory_, error);
            EXPECT_FALSE(error) << error.message();
        }
    }

    std::string ReadLog() const {
        std::ifstream stream(path_, std::ios::binary);
        EXPECT_TRUE(stream.is_open());
        return std::string(std::istreambuf_iterator<char>(stream),
                           std::istreambuf_iterator<char>());
    }

    std::string path_;

   private:
    std::filesystem::path directory_;
    bool owns_directory_ = false;
};

TEST_F(PlogTest, WritesFileWithMillisecondTimestamp) {
    {
        plog::RollingFileAppender<plog::TxtFormatter> appender(path_.c_str());
        plog::Logger<0> logger(plog::info);
        logger.addAppender(&appender);
        PLOG_INFO << "Timestamped file message";
    }

    std::string content = ReadLog();
    // The default UTF-8 converter prefixes a new file with a BOM.
    ASSERT_EQ(content.substr(0, 3), "\xEF\xBB\xBF");
    content.erase(0, 3);
    EXPECT_TRUE(std::regex_search(
        content,
        std::regex(
            R"(^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\.[0-9]{3} INFO )")));
    EXPECT_NE(content.find("Timestamped file message"), std::string::npos);
}

TEST_F(PlogTest, AppendsWithoutTruncatingExistingFile) {
    {
        std::ofstream stream(path_, std::ios::binary);
        ASSERT_TRUE(stream.is_open());
        stream << "Existing message\n";
        stream.close();
        ASSERT_FALSE(stream.fail());
    }
    {
        plog::RollingFileAppender<plog::TxtFormatter> appender(path_.c_str());
        plog::Logger<0> logger(plog::info);
        logger.addAppender(&appender);
        PLOG_INFO << "Appended message";
    }

    const std::string content = ReadLog();
    EXPECT_EQ(content.find("Existing message\n"), 0U);
    EXPECT_NE(content.find("Appended message"), std::string::npos);
}

TEST_F(PlogTest, FiltersMessagesBelowConfiguredSeverity) {
    {
        plog::RollingFileAppender<plog::TxtFormatter> appender(path_.c_str());
        plog::Logger<0> logger(plog::warning);
        logger.addAppender(&appender);
        PLOG_INFO << "Filtered info message";
        PLOG_ERROR << "Retained error message";
    }

    const std::string content = ReadLog();
    EXPECT_EQ(content.find("Filtered info message"), std::string::npos);
    EXPECT_NE(content.find("Retained error message"), std::string::npos);
}

}  // namespace
