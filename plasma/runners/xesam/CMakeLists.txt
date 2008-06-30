set(krunner_xesam_SRCS
    xesamrunner.cpp
)

kde4_add_plugin(krunner_xesam ${krunner_xesam_SRCS})
target_link_libraries(krunner_xesam ${KDE4_KIO_LIBS} plasma)

install(TARGETS krunner_xesam DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-xesam.desktop DESTINATION ${SERVICES_INSTALL_DIR})

