set( INTERFACE_INTROSPECTION_XML_FILES
   introspection/nm-manager-client.xml
   introspection/nm-device.xml
   introspection/nm-access-point.xml
   introspection/nm-device-802-11-wireless.xml
   introspection/nm-device-802-3-ethernet.xml
#   introspection/nm-exported-connection.xml
   introspection/nm-ip4-config.xml
#   introspection/nm-vpn-connection.xml
#   introspection/nm-vpn-manager.xml
#   introspection/nm-vpn-plugin.xml
   )

set( ADAPTER_INTROSPECTION_XML_FILES
#   introspection/nm-settings.xml
   )

set_source_files_properties( ${INTERFACE_INTROSPECTION_XML_FILES} ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )

set( UNUSED_XML
)


#QT4_ADD_DBUS_INTERFACES( FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES} )
set( FILES_TO_BUILD activeconnection.cpp
   nm-deviceinterface.cpp
   nm-vpn-connectioninterface.cpp
   nm-access-pointinterface.cpp 
   nm-exported-connectioninterface.cpp
   nm-vpn-managerinterface.cpp
   nm-device-802-11-wirelessinterface.cpp
   nm-ip4-configinterface.cpp
   nm-vpn-plugininterface.cpp
   nm-device-802-3-ethernetinterface.cpp
   nm-manager-clientinterface.cpp
)
kde4_add_library(networkmanagerdbus ${FILES_TO_BUILD})

TARGET_LINK_LIBRARIES(networkmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

