// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_SRC_APP_STARTUP_CONFIG_H_
#define VASLET_SRC_APP_STARTUP_CONFIG_H_

#include <cstddef>
#include <string>

namespace vaslet {
namespace app {

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
    std::string file = "/etc/vasletd.log";
    LogLevel level = LogLevel::kInfo;
    std::size_t max_file_size = 0;
    int max_files = 0;
};

struct VasletdConfig {
    std::string socket_path = "/tmp/vasletd.sock";
    std::string pid_file = "/tmp/vasletd.pid";
};

struct StartupConfig {
    LogConfig log;
    VasletdConfig vasletd;
};

struct StartupConfigResult {
    StartupConfig config;
    std::string error;
};

// Reads a UTF-8 JSON object with optional log and vasletd objects. Missing
// fields use the defaults above. Unknown fields are ignored; the first
// occurrence of a duplicate field is used. Rejects incorrect types, empty
// paths, embedded NUL bytes in known string values, raw NUL bytes, invalid log
// levels, and out-of-range rotation values. Relative paths use the working
// directory. Paths are loaded without checking or creating their targets.
// An empty error indicates success. Failure returns default config values and
// an error without input contents; the file and running configuration are
// untouched.
StartupConfigResult LoadStartupConfig(const std::string& path);

}  // namespace app
}  // namespace vaslet

#endif  // VASLET_SRC_APP_STARTUP_CONFIG_H_
