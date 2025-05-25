include_guard(GLOBAL)

find_program(CLANG_FORMAT clang-format)

function(format TARGET FILES)
  if(CLANG_FORMAT)
    block(SCOPE_FOR VARIABLES)
    set(MAX_TS 0)
    set(CHANGED_FILES)
    set(FORMAT_TARGET ${TARGET}_format)
    cmake_path(APPEND LOCKFILE ${CMAKE_CURRENT_BINARY_DIR} "format.lock")

    if(EXISTS ${LOCKFILE})
      file(READ ${LOCKFILE} MAX_TS)
    endif()

    foreach(FILE ${FILES})
      file(TIMESTAMP ${FILE} TS "%s")

      if(${TS} LESS_EQUAL ${MAX_TS})
        continue()
      endif()

      list(APPEND CHANGED_FILES ${FILE})
    endforeach()

    list(LENGTH CHANGED_FILES CHANGES)

    if(${CHANGES} GREATER 0)
      message(STATUS
        "Format[${PROJECT_NAME}.${TARGET}]: ${CHANGES} files changed"
      )

      add_custom_target(${FORMAT_TARGET}
        ALL
        COMMAND ${CLANG_FORMAT} -i ${CHANGED_FILES}
        VERBATIM
      )

      add_custom_command(
        TARGET ${FORMAT_TARGET}
        POST_BUILD
        COMMAND sh -c "echo $(date +%s) > ${LOCKFILE}"
        VERBATIM
      )
    endif()

    endblock()
  endif()
endfunction()
