cmake_minimum_required(VERSION 3.20)
project(MaliceRHI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to build tests
option(BUILD_TESTS "Build tests" ON)

# Add external libraries
add_subdirectory(external/volk)
add_subdirectory(external/glfw)

# Add MaliceRHI lib
add_subdirectory(code)

# Add tests
if(BUILD_TESTS)
    include(FetchContent)

    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
    )
    FetchContent_MakeAvailable(glm)

    enable_testing()
    add_subdirectory(tests/MaliceFwdRenderer/code/)
endif()
