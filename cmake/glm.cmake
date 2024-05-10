include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/icaven/glm.git
        GIT_TAG dca38025fba63bb9284023e6de55f756b9e37cec
)

FetchContent_MakeAvailable(glm)