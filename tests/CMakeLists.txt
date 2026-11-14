include(FetchContent)

set(BUILD_GMOCK OFF)
set(INSTALL_GTEST OFF)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    # GoogleTest v1.18.0; pin the commit for reproducible test builds.
    GIT_TAG 063de7e9578f82b369302001269680b4b1553359
)
FetchContent_MakeAvailable(googletest)

add_subdirectory(units)
