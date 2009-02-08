include_directories( ${CMAKE_SOURCE_DIR}/workspace/libs )

add_subdirectory( ifaces )
add_subdirectory( backends )
add_subdirectory( tests )

########### next target ###############

set(solidcontrol_LIB_SRCS
   frontendobject.cpp
   managerbase.cpp
   powermanager.cpp
   powermanager_randr.cpp
   networking.cpp
   networkmanager.cpp
   networkinterface.cpp
   networkcdmainterface.cpp
   networkgsminterface.cpp
   networkserialinterface.cpp
   networkipv4config.cpp
   wirednetworkinterface.cpp
   wirelessaccesspoint.cpp
   wirelessnetworkinterface.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   bluetoothsecurity.cpp
)

qt4_add_dbus_interface(solidcontrol_LIB_SRCS org.kde.Solid.Networking.xml org_kde_solid_networking)

kde4_add_library(solidcontrol SHARED ${solidcontrol_LIB_SRCS})

target_link_libraries(solidcontrol ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} solidcontrolifaces ${X11_Xrandr_LIB} ${X11_X11_LIB} ${QT_QTGUI_LIBRARY} )

set_target_properties(solidcontrol PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrol EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

pkg_check_modules (XRANDR_1_2   xrandr>=1.2)
macro_bool_to_01(XRANDR_1_2_FOUND HAS_RANDR_1_2)

configure_file(config-randr.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-randr.h )

########### install files ###############

install( FILES solidpowermanager.desktop solidnetworkmanager.desktop solidbluetoothmanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES powermanager.h networkmanager.h networkinterface.h wirednetworkinterface.h
    wirelessnetworkinterface.h wirelessaccesspoint.h bluetoothmanager.h bluetoothinterface.h
    bluetoothremotedevice.h bluetoothinputdevice.h bluetoothsecurity.h
    singletondefs.h networking.h networkipv4config.h solid_control_export.h 
    networkcdmainterface.h networkgsminterface.h networkserialinterface.h
    DESTINATION
${INCLUDE_INSTALL_DIR}/solid/control COMPONENT Devel)

