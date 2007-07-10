set(time_engine_SRCS
    timeengine.cpp
)

kde4_add_plugin(plasma_engine_time ${time_engine_SRCS})
target_link_libraries(plasma_engine_time ${KDE4_KDECORE_LIBS} plasma)

install(TARGETS plasma_engine_time DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-engine-time.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

