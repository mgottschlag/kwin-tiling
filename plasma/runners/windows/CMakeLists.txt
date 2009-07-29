set(krunner_windows_SRCS
    windowsrunner.cpp
)

kde4_add_plugin(krunner_windows ${krunner_windows_SRCS})
target_link_libraries(krunner_windows ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS krunner_windows DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-windows.desktop DESTINATION ${SERVICES_INSTALL_DIR})
