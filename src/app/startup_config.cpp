// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include "app/startup_config.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/filereadstream.h>

#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <string_view>
#include <utility>

namespace vaslet {
namespace app {
namespace {

struct FileCloser {
    void operator()(std::FILE* file) const noexcept { std::fclose(file); }
};

bool ParseLogLevel(std::string_view text, LogLevel* level) {
    constexpr std::string_view kNames[] = {"none", "fatal", "error",  "warning",
                                           "info", "debug", "verbose"};
    constexpr LogLevel kLevels[] = {LogLevel::kNone,   LogLevel::kFatal,
                                    LogLevel::kError,  LogLevel::kWarning,
                                    LogLevel::kInfo,   LogLevel::kDebug,
                                    LogLevel::kVerbose};
    for (std::size_t i = 0; i < sizeof(kNames) / sizeof(kNames[0]); ++i) {
        if (text == kNames[i]) {
            *level = kLevels[i];
            return true;
        }
    }
    return false;
}

}  // namespace

StartupConfigResult LoadStartupConfig(const std::string& path) {
    if (path.empty() || path.find('\0') != std::string::npos) {
        return {{},
                "Configuration file path must be non-empty and contain no NUL "
                "bytes."};
    }
    const std::unique_ptr<std::FILE, FileCloser> file(
        std::fopen(path.c_str(), "rb"));
    if (!file) {
        return {{}, "Failed to open startup configuration file."};
    }
    char buffer[4096];
    rapidjson::FileReadStream stream(file.get(), buffer, sizeof(buffer));
    rapidjson::Document document;
    document.ParseStream<rapidjson::kParseValidateEncodingFlag>(stream);

    // A raw NUL byte advances the stream; the EOF sentinel does not.
    bool has_nul = false;
    if (stream.Peek() == '\0') {
        const std::size_t end = stream.Tell();
        stream.Take();
        has_nul = stream.Tell() != end;
    }
    if (std::ferror(file.get())) {
        return {{}, "Failed to read startup configuration file."};
    }
    if (has_nul) {
        return {{}, "Startup configuration contains a NUL byte."};
    }
    if (document.HasParseError()) {
        return {{},
                "Invalid startup JSON at byte " +
                    std::to_string(document.GetErrorOffset()) + ": " +
                    rapidjson::GetParseError_En(document.GetParseError())};
    }
    if (!document.IsObject()) {
        return {{}, "Startup configuration must be a JSON object."};
    }

    StartupConfig config;
    if (const auto log_member = document.FindMember("log");
        log_member != document.MemberEnd()) {
        const auto& log = log_member->value;
        if (!log.IsObject()) {
            return {{}, "log must be a JSON object."};
        }

        if (const auto member = log.FindMember("file");
            member != log.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsString() || value.GetStringLength() == 0 ||
                std::memchr(value.GetString(), '\0', value.GetStringLength()) !=
                    nullptr) {
                return {
                    {},
                    "log.file must be a non-empty string without NUL bytes."};
            }
            config.log.file.assign(value.GetString(), value.GetStringLength());
        }
        if (const auto member = log.FindMember("level");
            member != log.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsString() ||
                !ParseLogLevel({value.GetString(), value.GetStringLength()},
                               &config.log.level)) {
                return {
                    {},
                    "log.level must be one of: none, fatal, error, warning, "
                    "info, debug, verbose."};
            }
        }
        if (const auto member = log.FindMember("max_file_size");
            member != log.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsUint64() ||
                value.GetUint64() > std::numeric_limits<std::size_t>::max() ||
                (value.GetUint64() != 0 && value.GetUint64() < 1000)) {
                return {
                    {},
                    "log.max_file_size must be zero or an integer of at least "
                    "1000 bytes that fits size_t."};
            }
            config.log.max_file_size =
                static_cast<std::size_t>(value.GetUint64());
        }
        if (const auto member = log.FindMember("max_files");
            member != log.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsUint64() ||
                value.GetUint64() > static_cast<unsigned int>(
                                        std::numeric_limits<int>::max())) {
                return {
                    {},
                    "log.max_files must be an integer between 0 and INT_MAX."};
            }
            config.log.max_files = static_cast<int>(value.GetUint64());
        }
    }
    if (const auto vasletd_member = document.FindMember("vasletd");
        vasletd_member != document.MemberEnd()) {
        const auto& vasletd = vasletd_member->value;
        if (!vasletd.IsObject()) {
            return {{}, "vasletd must be a JSON object."};
        }
        if (const auto member = vasletd.FindMember("socket_path");
            member != vasletd.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsString() || value.GetStringLength() == 0 ||
                std::memchr(value.GetString(), '\0', value.GetStringLength()) !=
                    nullptr) {
                return {
                    {},
                    "vasletd.socket_path must be a non-empty string without "
                    "NUL bytes."};
            }
            config.vasletd.socket_path.assign(value.GetString(),
                                              value.GetStringLength());
        }
        if (const auto member = vasletd.FindMember("pid_file");
            member != vasletd.MemberEnd()) {
            const auto& value = member->value;
            if (!value.IsString() || value.GetStringLength() == 0 ||
                std::memchr(value.GetString(), '\0', value.GetStringLength()) !=
                    nullptr) {
                return {{},
                        "vasletd.pid_file must be a non-empty string without "
                        "NUL bytes."};
            }
            config.vasletd.pid_file.assign(value.GetString(),
                                           value.GetStringLength());
        }
    }
    return {std::move(config), {}};
}

}  // namespace app
}  // namespace vaslet
