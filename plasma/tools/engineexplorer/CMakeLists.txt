set(plasmaengineexplorer_SRCS
    engineexplorer.cpp
    main.cpp
)

macro_optional_find_package(Soprano)
macro_log_feature(Soprano_FOUND "Soprano" "Soprano Libraries" "kdesupport" FALSE "" "Adds support for the Soprano data type to Plasma Engine Explorer.")

if (Soprano_FOUND)
   include_directories( ${SOPRANO_INCLUDE_DIR} )
   add_definitions( -DFOUND_SOPRANO=1 )
endif (Soprano_FOUND)

kde4_add_ui_files(plasmaengineexplorer_SRCS engineexplorer.ui)
kde4_add_executable(plasmaengineexplorer ${plasmaengineexplorer_SRCS})

target_link_libraries(plasmaengineexplorer plasma kworkspace  ${KDE4_KIO_LIBS})

if (Soprano_FOUND)
   target_link_libraries(plasmaengineexplorer ${SOPRANO_LIBRARIES})
endif (Soprano_FOUND)

install(TARGETS plasmaengineexplorer DESTINATION ${BIN_INSTALL_DIR})

