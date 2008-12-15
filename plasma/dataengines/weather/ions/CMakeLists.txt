set (ionlib_SRCS ion.cpp weatherutils.cpp)
kde4_add_library (weather_ion SHARED ${ionlib_SRCS})
target_link_libraries (weather_ion ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})
target_link_libraries (weather_ion LINK_INTERFACE_LIBRARIES ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS})

set_target_properties(weather_ion PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install (TARGETS weather_ion EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})

SET (ion_envcan_SRCS ion_envcan.cpp)
SET (ion_noaa_SRCS ion_noaa.cpp)
SET (ion_bbcukmet_SRCS ion_bbcukmet.cpp)

kde4_add_plugin(ion_envcan ${ion_envcan_SRCS})
kde4_add_plugin(ion_noaa ${ion_noaa_SRCS})
kde4_add_plugin(ion_bbcukmet ${ion_bbcukmet_SRCS})

target_link_libraries (ion_envcan weather_ion)
target_link_libraries (ion_noaa weather_ion)
target_link_libraries (ion_bbcukmet weather_ion)

INSTALL (FILES ion.h
               ion_export.h
               weatherutils.h
         DESTINATION ${INCLUDE_INSTALL_DIR}/plasma/weather COMPONENT Devel)

INSTALL (FILES includes/Ion
               includes/WeatherUtils
         DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Plasma/Weather COMPONENT Devel)

INSTALL (FILES ion-envcan.desktop ion-noaa.desktop ion-bbcukmet.desktop 
         DESTINATION ${SERVICES_INSTALL_DIR})

INSTALL (FILES weather_ion.desktop
         DESTINATION ${SERVICETYPES_INSTALL_DIR})

INSTALL (TARGETS ion_envcan ion_noaa ion_bbcukmet 
         DESTINATION ${PLUGIN_INSTALL_DIR})

