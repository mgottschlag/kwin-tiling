add_subdirectory(qtscript)

if(QT_QTWEBKIT_FOUND)
    add_subdirectory(webkit)
endif(QT_QTWEBKIT_FOUND)

if (PythonSupport_FOUND)
    macro_optional_add_subdirectory(python)
endif (PythonSupport_FOUND)

if(GOOGLEGADGETS_FOUND)
    macro_optional_add_subdirectory(google_gadgets)
endif(GOOGLEGADGETS_FOUND)

if(QEdje_FOUND)
    macro_optional_add_subdirectory(qedjescript)
endif(QEdje_FOUND)
