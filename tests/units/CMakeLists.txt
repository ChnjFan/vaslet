add_executable(vaslet_unit_tests
    cmdline_test.cpp plog_test.cpp startup_config_test.cpp
    "${PROJECT_SOURCE_DIR}/src/app/startup_config.cpp"
)
target_include_directories(vaslet_unit_tests PRIVATE
    "${PROJECT_SOURCE_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_BINARY_DIR}/generated"
)
target_include_directories(vaslet_unit_tests SYSTEM PRIVATE
    "${PROJECT_SOURCE_DIR}/thirdparty/cmdline"
    "${PROJECT_SOURCE_DIR}/thirdparty/rapidjson/include"
)
target_link_libraries(vaslet_unit_tests PRIVATE
    GTest::gtest_main vaslet_build_options plog::plog
)
# Verify that plog can be used under the server's language restrictions.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
    set_source_files_properties(plog_test.cpp startup_config_test.cpp
        "${PROJECT_SOURCE_DIR}/src/app/startup_config.cpp" PROPERTIES
        COMPILE_OPTIONS "-fno-exceptions;-fno-rtti"
    )
endif()

include(GoogleTest)
gtest_discover_tests(vaslet_unit_tests)
