if(NOT CLANGFORMAT_EXECUTABLE)
  set(CLANGFORMAT_EXECUTABLE clang-format)
endif()

if(NOT EXISTS ${CLANGFORMAT_EXECUTABLE})
  find_program(clangformat_executable_tmp ${CLANGFORMAT_EXECUTABLE})
  if(clangformat_executable_tmp)
    set(CLANGFORMAT_EXECUTABLE ${clangformat_executable_tmp})
    unset(clangformat_executable_tmp)
  else()
    message(FATAL_ERROR "ClangFormat: ${CLANGFORMAT_EXECUTABLE} not found! Aborting")
  endif()
endif()

add_custom_target(clangformat
  COMMAND
    git ls-files -- '*.cpp' '*.hpp' '*.h' | xargs
    ${CLANGFORMAT_EXECUTABLE}
    --Werror
    --dry-run
  WORKING_DIRECTORY
    ${CMAKE_SOURCE_DIR}
  COMMENT
    "Formatting ${prefix} with ${CLANGFORMAT_EXECUTABLE} ..."
)