project(plasma-containmentactions-switchwindow)

set(switchwindow_SRCS
    switch.cpp
)
kde4_add_ui_files(switchwindow_SRCS config.ui)

kde4_add_plugin(plasma_containmentactions_switchwindow ${switchwindow_SRCS})
target_link_libraries(plasma_containmentactions_switchwindow ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_switchwindow DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-switchwindow.desktop DESTINATION ${SERVICES_INSTALL_DIR})
