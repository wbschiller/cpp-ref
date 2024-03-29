 # Add a google test with support for gmock
function(add_gtest)
    cmake_parse_arguments(G_TEST "" "NAME" "SRCS;LIBS" ${ARGN})

    add_executable(${G_TEST_NAME} ${G_TEST_SRCS})
    if (DO_CLANG_TIDY)
      set_target_properties(${G_TEST_NAME} PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
    endif()

    target_include_directories(
        ${G_TEST_NAME}
        PUBLIC ${CPPREF_SRC_DIR}
    )

    target_link_libraries(${G_TEST_NAME}
        gtest
        gtest_main
        gmock
        gmock_main
        ${G_TEST_LIBS}
    )

    add_test(
        NAME ${G_TEST_NAME}
        COMMAND ${G_TEST_NAME}
    )
endfunction()
