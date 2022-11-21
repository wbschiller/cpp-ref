
cmake_minimum_required(VERSION 3.10)
project(cppref_examples CXX)

add_cppref_executable(
    NAME fileaccess
    SRCS fileaccess.cpp
    LIBS spdlog::spdlog
    )

add_cppref_executable(
    NAME json_ex
    SRCS json_ex.cpp
    LIBS spdlog::spdlog nlohmann_json::nlohmann_json
    )
    
add_cppref_executable(
    NAME logging
    SRCS logging.cpp
    LIBS spdlog::spdlog
    )

