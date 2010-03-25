project(StatusNotifierWatcher)

set(kded_statusnotifierwatcher_SRCS statusnotifierwatcher.cpp )

QT4_ADD_DBUS_ADAPTOR(kded_statusnotifierwatcher_SRCS     ${KDE4_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierWatcher.xml
                     statusnotifierwatcher.h StatusNotifierWatcher)


set(statusnotifieritem_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.StatusNotifierItem.xml)
set_source_files_properties(${statusnotifieritem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "systemtraytypedefs.h"
   CLASSNAME OrgKdeStatusNotifierItemInterface
)
QT4_ADD_DBUS_INTERFACE(kded_statusnotifierwatcher_SRCS ${statusnotifieritem_xml} statusnotifieritem_interface)

kde4_add_plugin(kded_statusnotifierwatcher ${kded_statusnotifierwatcher_SRCS})


target_link_libraries(kded_statusnotifierwatcher  ${KDE4_KIO_LIBS} )

install(TARGETS kded_statusnotifierwatcher  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES statusnotifierwatcher.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)

