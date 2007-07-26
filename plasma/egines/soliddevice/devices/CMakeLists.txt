include_directories(${CMAKE_SOURCE_DIR}/workspace/lib ${PLASMA_INCLUDE_DIR})

set(devices_engine_SRCS
    devicesengine.cpp
)

kde4_add_plugin(plasma_engine_devices ${devices_engine_SRCS})
target_link_libraries(plasma_engine_devices ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${PLASMA_LIBS} )

install(TARGETS plasma_engine_devices DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-devices.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

