# Steps to generate the dbus interface files
# 1) Place the NetworkManager introspection dbus xml in introspection/
# 2) Update the list of xml files below that we want to generate interfaces for
# 3) Enable dbus subdirectory in ../CMakeLists.txt
#    Make, in order to generate the interface sources
# 3.1) Add annotations to the xml so qdbusxml2cpp knows what type or typedef to use
#      Add typedefs as needed to generic-types.h
# 4)   Add the list of generated files needed for the plugin to FILES_TO_BUILD
# 6) Copy the working generated interface sources into the source dir
# 5) Edit the sources as required to get them to build
# 5.1) Fix the inclusion guards as the compiler ignores anything after a '-' so the compiler thinks 
#      headers have already been included
# Run do-hand-edits to update the diff of the interfaces vs what is generated; this speeds up regenerating them since you just have to apply the diff to make them compile


set( INTERFACE_INTROSPECTION_XML_FILES
   introspection/nm-manager-client.xml
   introspection/nm-device.xml
   introspection/nm-access-point.xml
   introspection/nm-device-wifi.xml
   introspection/nm-device-ethernet.xml
   #Solid doesn't know about connections
   #introspection/nm-exported-connection.xml
   introspection/nm-ip4-config.xml
   introspection/nm-dhcp4-config.xml
   introspection/nm-device-gsm.xml
   introspection/nm-device-cdma.xml
   introspection/nm-device-serial.xml
   )

set( ADAPTER_INTROSPECTION_XML_FILES
#   introspection/nm-settings.xml
   )

set_source_files_properties( ${INTERFACE_INTROSPECTION_XML_FILES} ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )

set( UNUSED_XML
)


set( FILES_TO_BUILD
   )
set ( UNUSED_FILES
   activeconnection.cpp
   nm-manager-clientinterface.cpp
   nm-deviceinterface.cpp
   nm-access-pointinterface.cpp 
   nm-device-802-3-ethernetinterface.cpp
   nm-device-802-11-wirelessinterface.cpp
   nm-ip4-configinterface.cpp
)

set( UNUSED_GENERATED_FILES
   nm-vpn-connectioninterface.cpp
   nm-exported-connectioninterface.cpp
   nm-vpn-managerinterface.cpp
   nm-vpn-plugininterface.cpp
)

QT4_ADD_DBUS_INTERFACES( FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES} )

kde4_add_library(networkmanagerdbus ${FILES_TO_BUILD})

TARGET_LINK_LIBRARIES(networkmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

