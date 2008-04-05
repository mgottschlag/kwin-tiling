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

set( FILES_TO_BUILD activeconnection.cpp )

QT4_ADD_DBUS_INTERFACES( FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES} )

kde4_add_library(networkmanagerdbus ${FILES_TO_BUILD})

TARGET_LINK_LIBRARIES(networkmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

