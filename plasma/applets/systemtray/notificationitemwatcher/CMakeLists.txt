project(NotificationItemWatcher)

set(kded_notificationitemwatcher_SRCS notificationitemwatcher.cpp )

QT4_ADD_DBUS_ADAPTOR(kded_notificationitemwatcher_SRCS     org.kde.NotificationItemWatcher.xml
                     notificationitemwatcher.h NotificationItemWatcher)


set(notificationitem_xml org.kde.NotificationItem.xml)
set_source_files_properties(${notificationitem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "../protocols/dbussystemtray/systemtraytypes.h"
   CLASSNAME OrgKdeNotificationItemInterface
)
QT4_ADD_DBUS_INTERFACE(kded_notificationitemwatcher_SRCS ${notificationitem_xml} notificationitem_interface)

kde4_add_plugin(kded_notificationitemwatcher ${kded_notificationitemwatcher_SRCS})


target_link_libraries(kded_notificationitemwatcher  ${KDE4_KIO_LIBS} )

install(TARGETS kded_notificationitemwatcher  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES notificationitemwatcher.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)

install(FILES org.kde.NotificationItemWatcher.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
