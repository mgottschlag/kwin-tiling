include_directories(${BLUEZ_DEFINITIONS} ${CMAKE_CURRENT_SOURCE_DIR}/../control)

add_subdirectory(tests)

set(solid_bluez_PART_SRCS
	bluezcalljob.cpp
	bluez-bluetoothmanager.cpp
	bluez-bluetoothinterface.cpp
	bluez-bluetoothinputdevice.cpp
	bluez-bluetoothremotedevice.cpp
	bluez-bluetoothsecurity.cpp
	bluez-bluetoothsecurityadaptor.cpp
)


kde4_add_library(solid_bluez_static STATIC ${solid_bluez_PART_SRCS})
target_link_libraries(solid_bluez_static ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} solidcontrolifaces)

kde4_add_plugin(solid_bluez module.cpp ${solid_bluez_PART_SRCS})

target_link_libraries(solid_bluez ${KDE4_KDECORE_LIBS} ${QT_QTXML_LIBRARY} solidcontrolifaces)
install(TARGETS solid_bluez DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_bluez.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

