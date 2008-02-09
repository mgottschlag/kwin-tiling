ADD_SUBDIRECTORY(ions)

SET (ionlib_SRCS ions/ion.cpp ions/formulas.cpp)
kde4_add_library (weather_ion SHARED ${ionlib_SRCS})
TARGET_LINK_LIBRARIES (weather_ion ${KDE4_KIO_LIBS} plasma)
set_target_properties(weather_ion PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
INSTALL (TARGETS weather_ion DESTINATION ${LIB_INSTALL_DIR})

SET(weather_SRCS weatherengine.cpp)
kde4_add_plugin(plasma_engine_weather ${weather_SRCS})
TARGET_LINK_LIBRARIES (plasma_engine_weather ${KDE4_KIO_LIBS} plasma weather_ion)
INSTALL (TARGETS plasma_engine_weather DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (FILES plasma-dataengine-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})
