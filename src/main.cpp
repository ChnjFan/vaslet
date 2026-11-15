// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.
//
// Entry point for the vasletd configuration server.
// Loads startup configuration and initializes logging before server startup.

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "app/startup_config.h"
#include "cmdline.h"
#include "common/log.h"
#include "vaslet_paths.h"
#include "vaslet_version.h"

namespace {

plog::Severity ToPlogSeverity(vaslet::app::LogLevel level) {
    switch (level) {
        case vaslet::app::LogLevel::kNone:
            return plog::none;
        case vaslet::app::LogLevel::kFatal:
            return plog::fatal;
        case vaslet::app::LogLevel::kError:
            return plog::error;
        case vaslet::app::LogLevel::kWarning:
            return plog::warning;
        case vaslet::app::LogLevel::kInfo:
            return plog::info;
        case vaslet::app::LogLevel::kDebug:
            return plog::debug;
        case vaslet::app::LogLevel::kVerbose:
            return plog::verbose;
    }
    return plog::none;
}

// Called once before starting worker threads. Check the path before handing
// logging to plog, whose file appender opens lazily and reports no I/O status.
bool InitializeLogging(const vaslet::app::LogConfig& config) {
    std::FILE* file = std::fopen(config.file.c_str(), "ab");
    if (file == nullptr) {
        std::fprintf(stderr, "vasletd: Failed to open log file '%s': %s\n",
                     config.file.c_str(), std::strerror(errno));
        return false;
    }
    if (std::fclose(file) != 0) {
        std::fprintf(stderr, "vasletd: Failed to close log file '%s': %s\n",
                     config.file.c_str(), std::strerror(errno));
        return false;
    }

    const int max_files = config.max_file_size == 0 ? 0 : config.max_files;
    static plog::RollingFileAppender<plog::TxtFormatter> appender(
        config.file.c_str(), config.max_file_size, max_files);
    plog::init(ToPlogSeverity(config.level), &appender);
    return true;
}

// Parses command-line arguments and returns the resolved configuration path.
// Prints help/version output or diagnostic errors and exits the program
// directly when parsing should terminate execution.
std::string ParseCommandLine(int argc, char* argv[]) {
    cmdline::parser parser;
    parser.set_program_name("vasletd");
    parser.add("version", 'v', "Print version and exit");
    parser.add("help", 'h', "Print help and exit");
    parser.add<std::string>("config", 'c', "Configuration file", false,
                            vaslet::kDefaultStartupConfigPath);
    if (!parser.parse(argc, argv)) {
        std::fprintf(stderr, "vasletd: %s\n", parser.error().c_str());
        std::fputs(parser.usage().c_str(), stderr);
        std::exit(EXIT_FAILURE);
    }
    if (!parser.rest().empty()) {
        std::fputs("vasletd: Unexpected positional argument.\n", stderr);
        std::exit(EXIT_FAILURE);
    }
    std::string config_path = parser.get<std::string>("config");
    if (config_path.empty()) {
        std::fputs("vasletd: Configuration path must not be empty.\n", stderr);
        std::exit(EXIT_FAILURE);
    }
    if (parser.exist("help")) {
        std::fputs(parser.usage().c_str(), stdout);
        std::exit(EXIT_SUCCESS);
    }
    if (parser.exist("version")) {
        std::printf("vasletd %s\n", vaslet::kVersion);
        std::exit(EXIT_SUCCESS);
    }
    return config_path;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string config_path = ParseCommandLine(argc, argv);
    const auto startup = vaslet::app::LoadStartupConfig(config_path);
    if (!startup.error.empty()) {
        std::fprintf(stderr, "vasletd: %s\n", startup.error.c_str());
        return EXIT_FAILURE;
    }
    if (!InitializeLogging(startup.config.log)) {
        return EXIT_FAILURE;
    }
    PLOG_INFO << "vasletd " << vaslet::kVersion << ": logging initialized";

    

    PLOG_INFO << "vasletd exiting";
    return EXIT_SUCCESS;
}
