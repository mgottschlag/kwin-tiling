set(hotplug_engine_SRCS
    hotplugengine.cpp
)

kde4_add_plugin(plasma_engine_hotplug ${hotplug_engine_SRCS})
target_link_libraries(plasma_engine_hotplug plasma ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_engine_hotplug DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-hotplug.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
