cmake_minimum_required(VERSION 3.16)
project(vaslet
    VERSION 0.1.0
    DESCRIPTION "A lightweight JSON configuration center for Linux"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

include(GNUInstallDirs)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND
   NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE AND
   NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT VASLET_IPO_SUPPORTED LANGUAGES CXX)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE "${VASLET_IPO_SUPPORTED}")
endif()

# ---------------------------------------------------------------------------
# Compiler warnings for project targets
# ---------------------------------------------------------------------------
add_library(vaslet_build_options INTERFACE)
if(MSVC)
    target_compile_options(vaslet_build_options INTERFACE /W4 /WX)
else()
    target_compile_options(vaslet_build_options INTERFACE
        -Wall -Wextra -Wpedantic -Werror
    )
endif()

# ---------------------------------------------------------------------------
# Server language policy and mandatory Release options
# ---------------------------------------------------------------------------
add_library(vaslet_server_release_options INTERFACE)
target_compile_definitions(vaslet_server_release_options INTERFACE
    "$<$<CONFIG:Release>:VASLETD_RELEASE_BUILD=1>"
)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    target_compile_options(vaslet_server_release_options INTERFACE
        -fno-exceptions -fno-rtti
        "$<$<CONFIG:Release>:-Os;-fvisibility=hidden;-fvisibility-inlines-hidden>"
    )
    # GCC and Clang expose no predefined macro for default symbol visibility.
    # Keep this build-contract marker coupled to the flags above.
    target_compile_definitions(vaslet_server_release_options INTERFACE
        "$<$<CONFIG:Release>:VASLETD_HIDDEN_VISIBILITY=1>"
    )
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/plog/include/plog/Log.h")
    message(FATAL_ERROR
        "plog is missing. Run: git submodule update --init thirdparty/plog"
    )
endif()
set(PLOG_BUILD_SAMPLES OFF CACHE BOOL "Build plog samples" FORCE)
set(PLOG_BUILD_TESTS OFF CACHE BOOL "Build plog tests" FORCE)
set(PLOG_INSTALL OFF CACHE BOOL "Install plog headers and CMake files" FORCE)
add_subdirectory(thirdparty/plog EXCLUDE_FROM_ALL)
set_property(TARGET plog APPEND PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
    "${PROJECT_SOURCE_DIR}/thirdparty/plog/include"
)

# ---------------------------------------------------------------------------
# Source modules
# ---------------------------------------------------------------------------
set(VASLET_INSTALL_CONFIGDIR "${CMAKE_INSTALL_SYSCONFDIR}/vaslet")
get_filename_component(VASLET_DEFAULT_STARTUP_CONFIG_PATH
    "${VASLET_INSTALL_CONFIGDIR}/vasletd_conf.json"
    ABSOLUTE BASE_DIR "${CMAKE_INSTALL_PREFIX}"
)
string(REPLACE "\\" "\\\\" VASLET_DEFAULT_STARTUP_CONFIG_PATH_ESCAPED
    "${VASLET_DEFAULT_STARTUP_CONFIG_PATH}")
string(REPLACE "\"" "\\\"" VASLET_DEFAULT_STARTUP_CONFIG_PATH_ESCAPED
    "${VASLET_DEFAULT_STARTUP_CONFIG_PATH_ESCAPED}")
configure_file(common/paths.h.in generated/vaslet_paths.h @ONLY)
configure_file(common/version.h.in generated/vaslet_version.h @ONLY)
file(GLOB_RECURSE VASLET_COMMON_SOURCES CONFIGURE_DEPENDS
    common/*.cpp
    common/*.h
)
add_subdirectory(src)

# ---------------------------------------------------------------------------
# Testing
# ---------------------------------------------------------------------------
option(BUILD_TESTING "Build the Vaslet test suite" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Examples
# ---------------------------------------------------------------------------
option(VASLET_BUILD_EXAMPLES "Build Vaslet examples" OFF)
if(VASLET_BUILD_EXAMPLES)
endif()

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
install(TARGETS vasletd
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES config/vasletd_conf.json DESTINATION "${VASLET_INSTALL_CONFIGDIR}")
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/vaslet"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    OPTIONAL
)
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DATADIR}/vaslet)
install(FILES thirdparty/rapidjson/license.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vaslet
    RENAME rapidjson-LICENSE.txt
)
install(FILES thirdparty/cmdline/LICENSE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vaslet
    RENAME cmdline-LICENSE.txt
)
install(FILES thirdparty/plog/LICENSE
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vaslet
    RENAME plog-LICENSE.txt
)
