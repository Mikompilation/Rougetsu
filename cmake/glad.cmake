include(FetchContent)

FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.6
        SOURCE_SUBDIR  cmake
)
set(glad_BUILD_TESTS OFF CACHE INTERNAL "")

FetchContent_MakeAvailable(glad)