set(plasmaengineexplorer_SRCS
    engineexplorer.cpp
    main.cpp
)

include_directories( ${SOPRANO_INCLUDE_DIR} )

kde4_add_ui_files(plasmaengineexplorer_SRCS engineexplorer.ui)
kde4_add_executable(plasmaengineexplorer ${plasmaengineexplorer_SRCS})

target_link_libraries(plasmaengineexplorer plasma kworkspace  ${KDE4_KIO_LIBS} ${SOPRANO_LIBRARIES})

install(TARGETS plasmaengineexplorer DESTINATION ${BIN_INSTALL_DIR})

