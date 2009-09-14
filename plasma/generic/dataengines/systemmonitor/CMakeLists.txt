
set(systemmonitor_engine_SRCS
   systemmonitor.cpp
)

kde4_add_plugin(plasma_engine_systemmonitor ${systemmonitor_engine_SRCS})
target_link_libraries(plasma_engine_systemmonitor ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} ksgrd)

install(TARGETS plasma_engine_systemmonitor DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-systemmonitor.desktop DESTINATION ${SERVICES_INSTALL_DIR} )




