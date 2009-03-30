project(NotificationAreaWatcher)

set(kded_notificationareawatcher_SRCS notificationareawatcher.cpp )

QT4_ADD_DBUS_ADAPTOR(kded_notificationareawatcher_SRCS     org.kde.NotificationAreaWatcher.xml
                     notificationareawatcher.h NotificationAreaWatcher)


set(notificationareaitem_xml org.kde.NotificationAreaItem.xml)
set_source_files_properties(${notificationareaitem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "../protocols/dbussystemtray/systemtraytypes.h"
   CLASSNAME OrgKdeNotificationAreaItemInterface
)
QT4_ADD_DBUS_INTERFACE(kded_notificationareawatcher_SRCS ${notificationareaitem_xml} notificationarea_interface)

kde4_add_plugin(kded_notificationareawatcher ${kded_notificationareawatcher_SRCS})


target_link_libraries(kded_notificationareawatcher  ${KDE4_KIO_LIBS} )

install(TARGETS kded_notificationareawatcher  DESTINATION ${PLUGIN_INSTALL_DIR})


install( FILES notificationareawatcher.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kded)

install(FILES org.kde.NotificationAreaWatcher.xml
        DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})
