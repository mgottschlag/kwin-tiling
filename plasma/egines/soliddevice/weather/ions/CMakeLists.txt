INCLUDE(${QT_USE_FILE})

INCLUDE_DIRECTORIES(
   ${CMAKE_SOURCE_DIR}
   ${QT_INCLUDE_DIR}
   ${KDE4_INCLUDES}
   ${PLASMA_INCLUDE_DIR}
)

SET (ion_envcan_SRCS ion_envcan.cpp)
SET (ion_noaa_SRCS ion_noaa.cpp)

kde4_add_plugin(ion_envcan ${ion_envcan_SRCS})
kde4_add_plugin(ion_noaa ${ion_noaa_SRCS})

TARGET_LINK_LIBRARIES (ion_envcan ${KDE4_KDEUI_LIBS} plasma weather_ion)
TARGET_LINK_LIBRARIES (ion_noaa ${KDE4_KDEUI_LIBS} plasma weather_ion)
INSTALL (TARGETS ion_envcan DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (TARGETS ion_noaa DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (FILES weather_ion.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
INSTALL (FILES ion-envcan.desktop DESTINATION ${SERVICES_INSTALL_DIR})
INSTALL (FILES ion-noaa.desktop DESTINATION ${SERVICES_INSTALL_DIR})
