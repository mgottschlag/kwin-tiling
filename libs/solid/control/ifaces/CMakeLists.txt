

########### next target ###############

set(solidcontrolifaces_LIB_SRCS 
   powermanager.cpp
   networkinterface.cpp
   networkcdmainterface.cpp
   networkgsminterface.cpp
   networkserialinterface.cpp
   networkmanager.cpp
   wirednetworkinterface.cpp
   wirelessaccesspoint.cpp
   wirelessnetworkinterface.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothsecurity.cpp
   remotecontrol.cpp
   remotecontrolmanager.cpp
   modemmanager.cpp
   modemmanagerinterface.cpp
   modeminterface.cpp
   modemlocationinterface.cpp
   modemcdmainterface.cpp
   modemgsmcardinterface.cpp
   modemgsmcontactsinterface.cpp
   modemgsmnetworkinterface.cpp
   modemgsmsmsinterface.cpp
   modemgsmhsointerface.cpp
   modemgsmussdinterface.cpp )

set(unused
   networkmanager.cpp
   modemmanager.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   remotecontrolmanager.cpp
   )

kde4_add_library(solidcontrolifaces SHARED ${solidcontrolifaces_LIB_SRCS})

target_link_libraries(solidcontrolifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidcontrolifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrolifaces EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS}  )


########### install files ###############

install( FILES powermanager.h networkmanager.h networkinterface.h networkcdmainterface.h networkgsminterface.h networkserialinterface.h wirednetworkinterface.h wirelessnetworkinterface.h wirelessaccesspoint.h bluetoothremotedevice.h bluetoothinputdevice.h bluetoothinterface.h bluetoothmanager.h remotecontrol.h remotecontrolmanager.h modemmanager.h modeminterface.h modemlocationinterface.h modemcdmainterface.h modemgsmcardinterface.h modemgsmcontactsinterface.h modemgsmnetworkinterface.h modemgsmsmsinterface.h modemgsmhsointerface.h modemgsmussdinterface.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/control/ifaces COMPONENT Devel)

