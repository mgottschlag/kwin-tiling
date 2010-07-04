# Steps to generate the dbus interface files
# 1) Place the ModemManager introspection dbus xml in introspection/
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

include_directories(
   ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs
)

set( INTERFACE_INTROSPECTION_XML_FILES
   introspection/mm-manager-client.xml
   introspection/mm-modem.xml
   introspection/mm-modem-cdma.xml
   introspection/mm-modem-location.xml
   introspection/mm-modem-simple.xml
   introspection/mm-modem-gsm-card.xml
   introspection/mm-modem-gsm-contacts.xml
   introspection/mm-modem-gsm-network.xml
   introspection/mm-modem-gsm-sms.xml
   introspection/mm-modem-gsm-hso.xml
   introspection/mm-modem-gsm-ussd.xml
   )

set( ADAPTER_INTROSPECTION_XML_FILES
   )

set_source_files_properties( ${INTERFACE_INTROSPECTION_XML_FILES} ${ADAPTER_INTROSPECTION_XML_FILES} PROPERTIES NO_NAMESPACE TRUE )

set( UNUSED_XML
)


set( FILES_TO_BUILD
   )
set ( UNUSED_FILES
   mm-manager-clientinterface.cpp
)

set( UNUSED_GENERATED_FILES
)

QT4_ADD_DBUS_INTERFACES( FILES_TO_BUILD ${INTERFACE_INTROSPECTION_XML_FILES} )

kde4_add_library(modemmanagerdbus ${FILES_TO_BUILD})

TARGET_LINK_LIBRARIES(modemmanagerdbus ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY})

