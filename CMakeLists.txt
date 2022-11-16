cmake_minimum_required(VERSION 3.14.0)

set(PROJECT_NAME CppRef)
project(${PROJECT_NAME} LANGUAGES CXX)

# Global C/C++ language options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CPPREF_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CPPREF_SRC_DIR ${CPPREF_ROOT_DIR}/src)

# Places we can `include` modules from.
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_LIST_DIR}/cmake"
)

# These options can be configured either in the `platform` CMakeLists files, or
# by passing a value to `-D` on the cmake command line.
option(CPPREF_ENABLE_SANITIZE "Enable address sanitizization" FALSE)
option(CPPREF_ENABLE_WERROR "Turn all warnings into errors" FALSE)
option(CPPREF_ENABLE_TIDY "Build with clang-tidy if it is installed" FALSE)
option(CPPREF_BUILD_EXAMPLE "Build example" FALSE)

if(CPPREF_ENABLE_WERROR)
  add_compile_options(-Werror)
endif()

if(CPPREF_ENABLE_TIDY)
  # Add clang-tidy target
  find_program(CLANG_TIDY_EXE NAMES clang-tidy)
  if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}" "--header-filter=src/*")
  else()
    message(STATUS "clang-tidy not found.")
  endif()
endif()

include(clang_format)

# Build googletest as part of this project
set(GTEST_HAS_PTHREAD 1)
set(GTEST_GMOCK ON)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        58d77fa8070e8cec2dc1ed015d66b454c8d78850 # release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(cppref_test)
add_subdirectory(doc)
add_subdirectory(extern)
add_subdirectory(src)

if(CPPREF_BUILD_EXAMPLE)
    message(STATUS "Generating example(s)")
    add_subdirectory(example)
endif()
