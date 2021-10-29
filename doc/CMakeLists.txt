add_custom_target(
  doc
  COMMAND doxygen Doxygen
  WORKING_DIRECTORY ${CPPREF_ROOT_DIR}/doc
  COMMENT "Generating API documentation with Doxygen"
  VERBATIM
)
