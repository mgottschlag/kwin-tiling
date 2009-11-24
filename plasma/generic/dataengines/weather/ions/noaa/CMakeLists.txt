SET (ion_noaa_SRCS ion_noaa.cpp)
kde4_add_plugin(ion_noaa ${ion_noaa_SRCS})
target_link_libraries (ion_noaa ${KDE4_SOLID_LIBS} weather_ion)

INSTALL (FILES ion-noaa.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (TARGETS ion_noaa DESTINATION ${PLUGIN_INSTALL_DIR})

