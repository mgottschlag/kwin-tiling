SET (ionlib_SRCS ion.cpp formulas.cpp)
kde4_add_library (weather_ion SHARED ${ionlib_SRCS})
TARGET_LINK_LIBRARIES (weather_ion ${KDE4_KIO_LIBS} plasma)
set_target_properties(weather_ion PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

kde4_target_link_interface_libraries(weather_ion "${KDE4_KIO_LIBS}")

INSTALL (TARGETS weather_ion ${INSTALL_TARGETS_DEFAULT_ARGS})

SET (ion_envcan_SRCS ion_envcan.cpp)
SET (ion_noaa_SRCS ion_noaa.cpp)
SET (ion_bbcukmet_SRCS ion_bbcukmet.cpp)

kde4_add_plugin(ion_envcan ${ion_envcan_SRCS})
kde4_add_plugin(ion_noaa ${ion_noaa_SRCS})
kde4_add_plugin(ion_bbcukmet ${ion_bbcukmet_SRCS})

TARGET_LINK_LIBRARIES (ion_envcan ${KDE4_KDEUI_LIBS} plasma weather_ion)
TARGET_LINK_LIBRARIES (ion_noaa ${KDE4_KDEUI_LIBS} plasma weather_ion)
TARGET_LINK_LIBRARIES (ion_bbcukmet ${KDE4_KDEUI_LIBS} plasma weather_ion)

INSTALL (FILES ion.h
               ion_export.h
               formulas.h
	 DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/weather COMPONENT Devel)

INSTALL (FILES includes/Ion
               includes/Formulas
	 DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma/Weather COMPONENT Devel)

INSTALL (TARGETS ion_envcan DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (TARGETS ion_noaa DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (TARGETS ion_bbcukmet DESTINATION ${PLUGIN_INSTALL_DIR})

INSTALL (FILES weather_ion.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
INSTALL (FILES ion-envcan.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL (FILES ion-noaa.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL (FILES ion-bbcukmet.desktop DESTINATION ${SERVICES_INSTALL_DIR})
