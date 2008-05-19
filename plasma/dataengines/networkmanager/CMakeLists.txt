include_directories(${CMAKE_SOURCE_DIR}/workspace/libs)

set(networkmanager_engine_SRCS
    networkmanagerengine.cpp
    networksignalmapper.cpp
    networksignalmapmanager.cpp
)

kde4_add_plugin(plasma_engine_networkmanager ${networkmanager_engine_SRCS})
target_link_libraries(plasma_engine_networkmanager ${KDE4_KDECORE_LIBS} ${PLASMA_LIBS} solidcontrol)

install(TARGETS plasma_engine_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
