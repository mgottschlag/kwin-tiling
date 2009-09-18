project(searchlaunchengine)

set(searchlaunch_engine_SRCS
        salengine.cpp
        salservice.cpp)

kde4_add_plugin(plasma_engine_searchlaunch ${searchlaunch_engine_SRCS})
target_link_libraries(plasma_engine_searchlaunch
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_engine_searchlaunch
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-engine-searchlaunch.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES searchlaunch.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
