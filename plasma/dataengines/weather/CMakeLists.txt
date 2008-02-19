ADD_SUBDIRECTORY(ions)

SET(weather_SRCS weatherengine.cpp)
kde4_add_plugin(plasma_engine_weather ${weather_SRCS})
TARGET_LINK_LIBRARIES (plasma_engine_weather ${KDE4_KIO_LIBS} plasma weather_ion)
INSTALL (TARGETS plasma_engine_weather DESTINATION ${PLUGIN_INSTALL_DIR})
INSTALL (FILES plasma-dataengine-weather.desktop DESTINATION ${SERVICES_INSTALL_DIR})
