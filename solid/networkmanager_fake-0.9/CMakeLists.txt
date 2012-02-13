include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
   ${NETWORKMANAGER_INCLUDE_DIRS}
   ${NM-UTIL_INCLUDE_DIRS}
   ${CMAKE_CURRENT_SOURCE_DIR}/dbus
   ${CMAKE_CURRENT_BUILD_DIR}
)

#add_subdirectory(dbus)
#add_subdirectory(fakemanager)

#add_subdirectory(tests)

set(solid_networkmanager_PART_SRCS
   #NetworkManager-network.cpp
   #NetworkManager-networkmanager.cpp
   #NetworkManager-networkinterface.cpp
   #NetworkManager-wirelessnetwork.cpp
   #NetworkManager-dbushelper.cpp
   manager.cpp
)

set( DBUS_INTERFACES_FILES
   dbus/nm-manager-clientinterface.cpp
)

kde4_add_library(solid_networkmanager09_fake_static STATIC ${solid_networkmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})
target_link_libraries(solid_networkmanager09_fake_static ${KDE4_KDECORE_LIBS} solidcontrolifaces)

#kde4_add_plugin(solid_networkmanager ${solid_networkmanager_PART_SRCS})
kde4_add_plugin(solid_networkmanager09_fake module.cpp ${solid_networkmanager_PART_SRCS} ${DBUS_INTERFACES_FILES})

set(NMPROPTEST_SRCS nmpropertyhelper.cpp nmpropertytest.cpp)
#kde4_add_executable(nmpropertytest ${NMPROPTEST_SRCS})
#target_link_libraries(nmpropertytest ${KDE4_KDECORE_LIBS})

target_link_libraries(solid_networkmanager09_fake ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol solidcontrolifaces)
install(TARGETS solid_networkmanager09_fake DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_networkmanager09_fake.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )
