
cmake_minimum_required(VERSION 3.10)
project(logging_examples CXX)

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()

# ---------------------------------------------------------------------------------------
# Example of using pre-compiled library
# ---------------------------------------------------------------------------------------
add_executable(logging logging.cpp)
target_link_libraries(logging PRIVATE spdlog::spdlog)

if (DO_CLANG_TIDY)
    set_target_properties(logging PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()
