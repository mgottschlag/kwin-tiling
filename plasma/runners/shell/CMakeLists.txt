set(krunner_shell_SRCS
    shellrunner.cpp
)

set(kcm_krunner_shell_SRCS
    shell_config.cpp
)


kde4_add_ui_files(kcm_krunner_shell_SRCS shellOptions.ui)

kde4_add_plugin(kcm_krunner_shell ${kcm_krunner_shell_SRCS})
target_link_libraries(kcm_krunner_shell
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

kde4_add_plugin(krunner_shell ${krunner_shell_SRCS})
target_link_libraries(krunner_shell ${KDE4_KIO_LIBS} ${PLASMA_LIBS})

install(TARGETS krunner_shell kcm_krunner_shell DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-shell.desktop plasma-runner-shell_config.desktop DESTINATION ${SERVICES_INSTALL_DIR})

