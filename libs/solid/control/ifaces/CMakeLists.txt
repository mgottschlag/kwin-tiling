

########### next target ###############

set(solidcontrolifaces_LIB_SRCS 
   powermanager.cpp
   networkmanager.cpp
   networkinterface.cpp
   network.cpp
   wirelessnetwork.cpp
   authentication.cpp
   bluetoothremotedevice.cpp
   bluetoothinputdevice.cpp
   bluetoothinterface.cpp
   bluetoothmanager.cpp
   bluetoothsecurity.cpp )


kde4_add_library(solidcontrolifaces SHARED ${solidcontrolifaces_LIB_SRCS})

target_link_libraries(solidcontrolifaces  ${KDE4_KDECORE_LIBS} )

set_target_properties(solidcontrolifaces PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrolifaces ${INSTALL_TARGETS_DEFAULT_ARGS}  )


########### install files ###############

install( FILES powermanager.h networkmanager.h networkinterface.h network.h wirelessnetwork.h authentication.h bluetoothremotedevice.h bluetoothinputdevice.h bluetoothinterface.h bluetoothmanager.h DESTINATION ${INCLUDE_INSTALL_DIR}/solid/control/ifaces)

