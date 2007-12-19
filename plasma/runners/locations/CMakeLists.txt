set(krunner_locations_SRCS
    locationrunner.cpp
)

kde4_add_plugin(krunner_locations ${krunner_locations_SRCS})
target_link_libraries(krunner_locations ${KDE4_KIO_LIBS} plasma)

install(TARGETS krunner_locations DESTINATION ${PLUGIN_INSTALL_DIR} )

install(FILES plasma-runner-locations.desktop DESTINATION ${SERVICES_INSTALL_DIR})

