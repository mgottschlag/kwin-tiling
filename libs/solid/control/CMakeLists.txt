add_subdirectory( ifaces )

########### next target ###############

set(solidcontrol_LIB_SRCS 
   frontendobject.cpp
   managerbase.cpp
   powermanager.cpp
   networkmanager.cpp
   networkinterface.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   networking.cpp
)

qt4_add_dbus_interface(solidcontrol_LIB_SRCS org.kde.Solid.Networking.xml org_kde_solid_networking)

kde4_automoc(${solidcontrol_LIB_SRCS})

kde4_add_library(solidcontrol SHARED ${solidcontrol_LIB_SRCS})

target_link_libraries(solidcontrol ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrolifaces )

set_target_properties(solidcontrol PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrol DESTINATION ${LIB_INSTALL_DIR} )


########### install files ###############

install( FILES solidpowermanager.desktop solidnetworkmanager.desktop solidbluetoothmanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES powermanager.h networkmanager.h networkinterface.h network.h wirelessnetwork.h
    authentication.h bluetoothmanager.h bluetoothinterface.h bluetoothremotedevice.h
    bluetoothinputdevice.h singletondefs.h networking.h solid_control_export.h DESTINATION 
${INCLUDE_INSTALL_DIR}/solid/control)

