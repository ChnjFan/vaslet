// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_APP_STARTUP_CONFIG_H_
#define VASLET_SRC_APP_STARTUP_CONFIG_H_

#include <cstddef>
#include <string>

namespace vaslet {

enum class LogLevel {
    kNone,
    kFatal,
    kError,
    kWarning,
    kInfo,
    kDebug,
    kVerbose
};

struct LogConfig {
    std::string file = "vasletd.log";
    LogLevel level = LogLevel::kInfo;
    std::size_t max_file_size = 0;
    int max_files = 0;
};

struct StartupConfig {
    LogConfig log;
};

struct StartupConfigResult {
    StartupConfig config;
    std::string error;
};

// Reads a UTF-8 JSON object. Missing fields use the defaults above. Rejects
// unknown/duplicate fields, incorrect types, embedded NUL bytes, invalid
// levels, and out-of-range rotation values. Relative log paths use the working
// directory. An empty error indicates success. Failure returns default config
// values and an error without input contents; the file and running
// configuration are untouched.
StartupConfigResult LoadStartupConfig(const std::string& path);

}  // namespace vaslet

#endif  // VASLET_SRC_APP_STARTUP_CONFIG_H_
