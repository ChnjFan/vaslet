// Copyright (c) 2026 ChnjFan
// Licensed under the MIT License. See LICENSE in the project root.

#ifndef VASLET_COMMON_LOG_H_
#define VASLET_COMMON_LOG_H_

// Project entry point for plog. Include this header instead of upstream
// headers. Use PLOG_INFO, PLOG_WARNING, and PLOG_ERROR for stream-style
// logging.
#include <plog/Appenders/RollingFileAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#endif  // VASLET_COMMON_LOG_H_
