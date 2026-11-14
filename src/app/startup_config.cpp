// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#include "app/startup_config.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include <string_view>
#include <utility>

namespace vaslet {
namespace {

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
    const std::unique_ptr<std::FILE, decltype(&std::fclose)> file(
        std::fopen(path.c_str(), "rb"), &std::fclose);
    if (!file) {
        return {{}, "Failed to open startup configuration file."};
    }
    std::string input;
    char buffer[4096];
    while (const std::size_t count =
               std::fread(buffer, 1, sizeof(buffer), file.get())) {
        if (std::memchr(buffer, '\0', count) != nullptr) {
            return {{}, "Startup configuration contains a NUL byte."};
        }
        input.append(buffer, count);
    }
    if (std::ferror(file.get())) {
        return {{}, "Failed to read startup configuration file."};
    }

    rapidjson::Document document;
    document.Parse<rapidjson::kParseValidateEncodingFlag>(input.data(),
                                                          input.size());
    if (document.HasParseError()) {
        return {{},
                "Invalid startup JSON at byte " +
                    std::to_string(document.GetErrorOffset()) + ": " +
                    rapidjson::GetParseError_En(document.GetParseError())};
    }
    if (!document.IsObject()) {
        return {{}, "Startup configuration must be a JSON object."};
    }

    const rapidjson::Value* log = nullptr;
    for (const auto& member : document.GetObject()) {
        const std::string_view name(member.name.GetString(),
                                    member.name.GetStringLength());
        if (name != "log") {
            return {{}, "Unknown startup configuration field."};
        }
        if (log != nullptr) {
            return {{}, "Duplicate startup configuration field: log."};
        }
        log = &member.value;
    }
    if (log == nullptr) {
        return {};
    }
    if (!log->IsObject()) {
        return {{}, "log must be a JSON object."};
    }

    StartupConfig config;
    unsigned int seen_fields = 0;
    for (const auto& member : log->GetObject()) {
        const std::string_view name(member.name.GetString(),
                                    member.name.GetStringLength());
        unsigned int field = 0;
        if (name == "file") {
            field = 1U;
        } else if (name == "level") {
            field = 2U;
        } else if (name == "max_file_size") {
            field = 4U;
        } else if (name == "max_files") {
            field = 8U;
        } else {
            return {{}, "Unknown log configuration field."};
        }
        if ((seen_fields & field) != 0) {
            return {{}, "Duplicate log configuration field."};
        }
        seen_fields |= field;
        const auto& value = member.value;
        switch (field) {
            case 1U:
                if (!value.IsString() || value.GetStringLength() == 0 ||
                    std::memchr(value.GetString(), '\0',
                                value.GetStringLength()) != nullptr) {
                    return {{},
                            "log.file must be a non-empty string without NUL "
                            "bytes."};
                }
                config.log.file.assign(value.GetString(),
                                       value.GetStringLength());
                break;
            case 2U:
                if (!value.IsString() ||
                    !ParseLogLevel({value.GetString(), value.GetStringLength()},
                                   &config.log.level)) {
                    return {{},
                            "log.level must be one of: none, fatal, error, "
                            "warning, info, debug, verbose."};
                }
                break;
            case 4U:
                if (!value.IsUint64() ||
                    value.GetUint64() >
                        std::numeric_limits<std::size_t>::max() ||
                    (value.GetUint64() != 0 && value.GetUint64() < 1000)) {
                    return {{},
                            "log.max_file_size must be zero or an integer of "
                            "at least 1000 bytes that fits size_t."};
                }
                config.log.max_file_size =
                    static_cast<std::size_t>(value.GetUint64());
                break;
            case 8U:
                if (!value.IsUint64() ||
                    value.GetUint64() > static_cast<unsigned int>(
                                            std::numeric_limits<int>::max())) {
                    return {{},
                            "log.max_files must be an integer between 0 and "
                            "INT_MAX."};
                }
                config.log.max_files = static_cast<int>(value.GetUint64());
                break;
        }
    }
    return {std::move(config), {}};
}

}  // namespace vaslet
