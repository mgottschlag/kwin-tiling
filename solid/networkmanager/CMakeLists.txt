add_subdirectory(tests)

include_directories(/usr/include/NetworkManager)

set(solid_networkmanager_PART_SRCS
	NetworkManager-networkmanager.cpp
	NetworkManager-networkinterface.cpp
)

kde4_automoc(${solid_networkmanager_PART_SRCS})

kde4_add_library(solid_networkmanager_static STATIC ${solid_networkmanager_PART_SRCS})
target_link_libraries(solid_networkmanager_static solidifaces ${KDE4_KDECORE_LIBS} ${KDE4_SOLIDIFACES_LIBS})

kde4_add_plugin(solid_networkmanager module.cpp ${solid_networkmanager_PART_SRCS})

target_link_libraries(solid_networkmanager ${KDE4_KDECORE_LIBS} ${KDE4_SOLIDIFACES_LIBS})
install(TARGETS solid_networkmanager DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_networkmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

