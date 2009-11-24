set(ion_wettercom_SRCS ion_wettercom.cpp)
kde4_add_plugin(ion_wettercom ${ion_wettercom_SRCS})
target_link_libraries(ion_wettercom weather_ion ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KUNITCONVERSION_LIBS})

install(FILES ion-wettercom.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS ion_wettercom DESTINATION ${PLUGIN_INSTALL_DIR})

