include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${NETWORKMANAGER_INCLUDE_DIRS} 
   ${CMAKE_CURRENT_SOURCE_DIR}/../control)

#add_subdirectory(tests) # disabled for now, they depend on authentication.cpp

set(solid_networkmanager_PART_SRCS
	NetworkManager-wirelessaccesspoint.cpp
	NetworkManager-networkmanager.cpp
	NetworkManager-networkinterface.cpp
	NetworkManager-wirelessnetwork.cpp
	NetworkManager-wirednetwork.cpp
#	NetworkManager-dbushelper.cpp
)


kde4_add_library(solid_networkmanager_static STATIC ${solid_networkmanager_PART_SRCS})
target_link_libraries(solid_networkmanager_static ${KDE4_KDECORE_LIBS} solidcontrolifaces)

kde4_add_plugin(solid_networkmanager module.cpp ${solid_networkmanager_PART_SRCS})

target_link_libraries(solid_networkmanager ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol solidcontrolifaces)
install(TARGETS solid_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
