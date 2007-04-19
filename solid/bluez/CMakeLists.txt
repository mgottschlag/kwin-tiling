include_directories(${BLUEZ_DEFINITIONS})

add_subdirectory(tests)

set(solid_bluez_PART_SRCS
	bluezcalljob.cpp
	bluez-bluetoothmanager.cpp
	bluez-bluetoothinterface.cpp
	bluez-bluetoothinputdevice.cpp
	bluez-bluetoothremotedevice.cpp
)

kde4_automoc(${solid_bluez_PART_SRCS})

kde4_add_library(solid_bluez_static STATIC ${solid_bluez_PART_SRCS})
target_link_libraries(solid_bluez_static ${KDE4_KDECORE_LIBS} solidexperimentalifaces)

kde4_add_plugin(solid_bluez module.cpp ${solid_bluez_PART_SRCS})

target_link_libraries(solid_bluez ${KDE4_KDECORE_LIBS} solidexperimentalifaces)
install(TARGETS solid_bluez DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_bluez.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
