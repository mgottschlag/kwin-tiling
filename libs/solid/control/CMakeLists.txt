kde4_no_enable_final(solid)

include_directories( ${CMAKE_SOURCE_DIR}/workspace/libs )

add_subdirectory( ifaces )
add_subdirectory( backends )
add_subdirectory( tests )

########### next target ###############

set(solidcontrol_LIB_SRCS
   frontendobject.cpp
   managerbase.cpp
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
   remotecontrolbutton.cpp
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
   modemgsmussdinterface.cpp
)

qt4_add_dbus_interface(solidcontrol_LIB_SRCS org.kde.Solid.Networking.xml org_kde_solid_networking)

kde4_add_library(solidcontrol SHARED ${solidcontrol_LIB_SRCS})

target_link_libraries(solidcontrol ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${QT_QTNETWORK_LIBRARY} solidcontrolifaces ${X11_Xrandr_LIB} ${X11_X11_LIB} ${QT_QTGUI_LIBRARY} )

set_target_properties(solidcontrol PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS solidcontrol EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES solidnetworkmanager.desktop solidremotecontrolmanager.desktop solidmodemmanager.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR}  )
install( FILES networkmanager.h networkinterface.h remotecontrolmanager.h
    remotecontrol.h remotecontrolbutton.h wirednetworkinterface.h wirelessnetworkinterface.h
    wirelessaccesspoint.h singletondefs.h networking.h networkipv4config.h solid_control_export.h 
    networkcdmainterface.h networkgsminterface.h networkserialinterface.h
    modemmanager.h modemmanagerinterface.h
    modeminterface.h modemlocationinterface.h modemcdmainterface.h
    modemgsmcardinterface.h modemgsmcontactsinterface.h
    modemgsmnetworkinterface.h modemgsmsmsinterface.h
    modemgsmhsointerface.h modemgsmussdinterface.h
    DESTINATION
${INCLUDE_INSTALL_DIR}/solid/control COMPONENT Devel)

