cmake_minimum_required(VERSION 3.14.0)

project(CppRef CXX)

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

if(CPPREF_ENABLE_WERROR)
  add_compile_options(-Werror)
endif()

# Build googletest as part of this project
set(GTEST_HAS_PTHREAD 1)
set(GTEST_GMOCK ON)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        e2239ee6043f73722e7aa812a459f54a28552929 # release-1.11.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(cppref_test)
add_subdirectory(src)
