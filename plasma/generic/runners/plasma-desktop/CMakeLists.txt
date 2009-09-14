set(krunner_plasma-desktop_SRCS
    plasma-desktop-runner.cpp
)

kde4_add_plugin(krunner_plasma-desktop ${krunner_plasma-desktop_SRCS})
target_link_libraries(krunner_plasma-desktop ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS krunner_plasma-desktop DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-plasma-desktop.desktop DESTINATION ${SERVICES_INSTALL_DIR})

