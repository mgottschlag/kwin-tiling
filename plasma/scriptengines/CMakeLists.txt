add_subdirectory(qtscript)
add_subdirectory(webkit)

if (PythonSupport_FOUND)
   #    macro_optional_add_subdirectory(python)
endif (PythonSupport_FOUND)

if(GOOGLEGADGETS_FOUND)
      macro_optional_add_subdirectory(google_gadgets)
endif(GOOGLEGADGETS_FOUND)

if(QEdje_FOUND)
  macro_optional_add_subdirectory(qedjescript)
endif(QEdje_FOUND)
