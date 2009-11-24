set(krunner_solid_SRCS
    solidrunner.cpp
    devicewrapper.cpp
)

kde4_add_plugin(krunner_solid ${krunner_solid_SRCS})
target_link_libraries(krunner_solid ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS krunner_solid DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-solid.desktop DESTINATION ${SERVICES_INSTALL_DIR})
