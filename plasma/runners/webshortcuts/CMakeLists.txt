set(krunner_webshortcuts_SRCS
    webshortcutrunner.cpp
)

kde4_add_plugin(krunner_webshortcuts ${krunner_webshortcuts_SRCS})
target_link_libraries(krunner_webshortcuts ${KDE4_KIO_LIBS} plasma)

install(TARGETS krunner_webshortcuts DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES krunner_webshortcuts.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

