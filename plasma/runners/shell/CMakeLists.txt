set(krunner_shell_SRCS
    shellrunner.cpp
)

kde4_add_ui_files(krunner_shell_SRCS shellOptions.ui)
kde4_add_plugin(krunner_shell ${krunner_shell_SRCS})
target_link_libraries(krunner_shell ${KDE4_KIO_LIBS} plasma)

install(TARGETS krunner_shell DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-shell.desktop DESTINATION ${SERVICES_INSTALL_DIR})

