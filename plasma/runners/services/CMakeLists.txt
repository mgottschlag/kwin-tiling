set(krunner_services_SRCS
    servicerunner.cpp
)

kde4_add_plugin(krunner_services ${krunner_services_SRCS})
target_link_libraries(krunner_services ${KDE4_KIO_LIBS} plasma)

install(TARGETS krunner_services DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-services.desktop DESTINATION ${SERVICES_INSTALL_DIR})

