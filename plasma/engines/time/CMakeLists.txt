
########### next target ###############

include_directories(${CMAKE_SOURCE_DIR}/workspace/plasma/lib)

set(time_engine_SRCS
    timeengine.cpp
)

kde4_automoc(${time_engine_SRCS})
kde4_add_plugin(plasma_time_engine ${time_engine_SRCS})
target_link_libraries(plasma_time_engine ${KDE4_KDECORE_LIBS} plasmoid)

install(TARGETS plasma_time_engine DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES plasma_time_engine.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

