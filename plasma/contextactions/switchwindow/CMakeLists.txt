project(plasma-contextaction-switchwindow)

set(switchwindow_SRCS
    switch.cpp
)
kde4_add_ui_files(switchwindow_SRCS config.ui)

kde4_add_plugin(plasma_contextaction_switchwindow ${switchwindow_SRCS})
target_link_libraries(plasma_contextaction_switchwindow ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_switchwindow DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-switchwindow.desktop DESTINATION ${SERVICES_INSTALL_DIR})
