# ---------------------------------------------------------------------------
# Configuration service
# ---------------------------------------------------------------------------
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include/rapidjson/document.h")
    message(FATAL_ERROR
        "RapidJSON is missing. Run: git submodule update --init thirdparty/rapidjson"
    )
endif()
file(GLOB_RECURSE VASLET_SERVER_SOURCES CONFIGURE_DEPENDS
    *.cpp
    *.h
)
add_executable(vasletd ${VASLET_SERVER_SOURCES} ${VASLET_COMMON_SOURCES})
target_include_directories(vasletd PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}/generated"
)
target_include_directories(vasletd SYSTEM PRIVATE
    "${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include"
    "${PROJECT_SOURCE_DIR}/thirdparty/cmdline"
)
# The unmodified cmdline header requires exceptions and RTTI. Enable them only
# for the entry point that registers and parses command-line options.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    set_source_files_properties(main.cpp PROPERTIES
        COMPILE_OPTIONS "-fexceptions;-frtti"
    )
endif()
target_link_libraries(vasletd PRIVATE
    vaslet_build_options vaslet_server_release_options plog::plog
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package(Threads REQUIRED)
    target_link_libraries(vasletd PRIVATE Threads::Threads)
endif()
