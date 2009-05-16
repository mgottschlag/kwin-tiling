set (time_solar_SRCS solarposition.cpp)
kde4_add_library (time_solar SHARED ${time_solar_SRCS})
target_link_libraries (time_solar ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})
target_link_libraries (time_solar LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS})

set_target_properties(time_solar PROPERTIES
   VERSION ${GENERIC_LIB_VERSION}
   SOVERSION ${GENERIC_LIB_SOVERSION}
)

install (TARGETS time_solar EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})


set(time_engine_SRCS
    timeengine.cpp
    timesource.cpp
    phases.cpp
)

kde4_add_plugin(plasma_engine_time ${time_engine_SRCS})
target_link_libraries(plasma_engine_time time_solar)

install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-time.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

