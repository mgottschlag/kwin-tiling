SET (ion_bbcukmet_SRCS ion_bbcukmet.cpp)
kde4_add_plugin(ion_bbcukmet ${ion_bbcukmet_SRCS})
target_link_libraries (ion_bbcukmet ${KDE4_SOLID_LIBS} weather_ion)

INSTALL (FILES ion-bbcukmet.desktop DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (TARGETS ion_bbcukmet DESTINATION ${PLUGIN_INSTALL_DIR})

