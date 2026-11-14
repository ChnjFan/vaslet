// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include "cmdline.h"

#include <gtest/gtest.h>

#include <string>

#include "vaslet_paths.h"

namespace {

// Keep the cmdline API behavior used by vasletd covered before any future
// reduction of the upstream library. Application options remain in main.cpp.
class CmdlineTest : public testing::Test {
   protected:
    void SetUp() override {
        parser_.add("version", 'v', "Print version and exit");
        parser_.add("help", 'h', "Print help and exit");
        parser_.add<std::string>("config", 'c', "Configuration file", false,
                                 vaslet::kDefaultStartupConfigPath);
    }

    cmdline::parser parser_;
};

TEST_F(CmdlineTest, UsesDefaultConfigurationPath) {
    const char* argv[] = {"vasletd"};

    ASSERT_TRUE(parser_.parse(1, argv)) << parser_.error();
    EXPECT_EQ(parser_.get<std::string>("config"),
              vaslet::kDefaultStartupConfigPath);
    EXPECT_FALSE(parser_.exist("config"));
    EXPECT_FALSE(parser_.exist("version"));
    EXPECT_FALSE(parser_.exist("help"));
}

TEST_F(CmdlineTest, ParsesVersionFlag) {
    const char* argv[] = {"vasletd", "-v"};

    ASSERT_TRUE(parser_.parse(2, argv)) << parser_.error();
    EXPECT_TRUE(parser_.exist("version"));
    EXPECT_FALSE(parser_.exist("help"));
}

TEST_F(CmdlineTest, ParsesHelpFlag) {
    const char* argv[] = {"vasletd", "-h"};

    ASSERT_TRUE(parser_.parse(2, argv)) << parser_.error();
    EXPECT_TRUE(parser_.exist("help"));
    EXPECT_FALSE(parser_.exist("version"));
}

TEST_F(CmdlineTest, ParsesExplicitConfigurationPath) {
    const char* argv[] = {"vasletd", "-c", "custom.json"};

    ASSERT_TRUE(parser_.parse(3, argv)) << parser_.error();
    EXPECT_TRUE(parser_.exist("config"));
    EXPECT_EQ(parser_.get<std::string>("config"), "custom.json");
}

TEST_F(CmdlineTest, PreservesSpacesInConfigurationPath) {
    const char* argv[] = {"vasletd", "-c", "space dir/config.json"};

    ASSERT_TRUE(parser_.parse(3, argv)) << parser_.error();
    EXPECT_EQ(parser_.get<std::string>("config"), "space dir/config.json");
}

TEST_F(CmdlineTest, UsesLastRepeatedConfigurationPath) {
    const char* argv[] = {"vasletd", "-c", "first.json", "-c", "last.json"};

    ASSERT_TRUE(parser_.parse(5, argv)) << parser_.error();
    EXPECT_EQ(parser_.get<std::string>("config"), "last.json");
}

TEST_F(CmdlineTest, ParsesGroupedShortFlags) {
    const char* argv[] = {"vasletd", "-vh"};

    ASSERT_TRUE(parser_.parse(2, argv)) << parser_.error();
    EXPECT_TRUE(parser_.exist("version"));
    EXPECT_TRUE(parser_.exist("help"));
}

TEST_F(CmdlineTest, ParsesLongOptions) {
    const char* argv[] = {"vasletd", "--version", "--help",
                          "--config=custom.json"};

    ASSERT_TRUE(parser_.parse(4, argv)) << parser_.error();
    EXPECT_TRUE(parser_.exist("version"));
    EXPECT_TRUE(parser_.exist("help"));
    EXPECT_EQ(parser_.get<std::string>("config"), "custom.json");
}

TEST_F(CmdlineTest, RejectsUnknownOption) {
    const char* argv[] = {"vasletd", "-x"};

    EXPECT_FALSE(parser_.parse(2, argv));
    EXPECT_FALSE(parser_.error().empty());
}

TEST_F(CmdlineTest, RejectsMissingConfigurationValue) {
    const char* argv[] = {"vasletd", "-c"};

    EXPECT_FALSE(parser_.parse(2, argv));
    EXPECT_FALSE(parser_.error().empty());
}

TEST_F(CmdlineTest, ExposesPositionalArgumentsForApplicationValidation) {
    const char* argv[] = {"vasletd", "extra.json"};

    ASSERT_TRUE(parser_.parse(2, argv)) << parser_.error();
    ASSERT_EQ(parser_.rest().size(), 1U);
    EXPECT_EQ(parser_.rest().front(), "extra.json");
}

}  // namespace
