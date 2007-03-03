include_directories(${NETWORKMANAGER_DEFINITIONS})

add_subdirectory(tests)

set(solid_networkmanager_PART_SRCS
	NetworkManager-network.cpp
	NetworkManager-networkmanager.cpp
	NetworkManager-networkinterface.cpp
	NetworkManager-wirelessnetwork.cpp
)

kde4_automoc(${solid_networkmanager_PART_SRCS})

kde4_add_library(solid_networkmanager_static STATIC ${solid_networkmanager_PART_SRCS})
target_link_libraries(solid_networkmanager_static ${KDE4_KDECORE_LIBS} ${KDE4_SOLIDIFACES_LIBS})

kde4_add_plugin(solid_networkmanager module.cpp ${solid_networkmanager_PART_SRCS})

target_link_libraries(solid_networkmanager ${KDE4_KDECORE_LIBS} ${KDE4_SOLIDIFACES_LIBS})
install(TARGETS solid_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
