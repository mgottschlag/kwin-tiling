add_subdirectory(webkit)
add_subdirectory(qscript)


if(GOOGLEGADGETS_FOUND)
      macro_optional_add_subdirectory(google_gadgets)
endif(GOOGLEGADGETS_FOUND)

if(QEdje_FOUND)
  macro_optional_add_subdirectory(qedjescript)
endif(QEdje_FOUND)