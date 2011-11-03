set(network_engine_SRCS
    networkengine.cpp
)

kde4_add_plugin(plasma_engine_network ${network_engine_SRCS})
target_link_libraries(plasma_engine_network ${KDE4_KDECORE_LIBS} ${KDE4_PLASMA_LIBS} solidcontrol)

install(TARGETS plasma_engine_network DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-network.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

