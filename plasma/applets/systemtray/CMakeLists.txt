project(plasma-systemtray)

set(systemtray_SRCS

    core/extendertask.cpp
    core/manager.cpp
    core/protocol.cpp
    core/task.cpp
    core/notification.cpp
    core/job.cpp

    protocols/fdo/fdoprotocol.cpp
    protocols/fdo/fdotask.cpp
    protocols/fdo/fdonotification.cpp
    protocols/fdo/fdographicswidget.cpp
    protocols/fdo/fdoselectionmanager.cpp
    protocols/fdo/x11embedcontainer.cpp
    protocols/fdo/x11embeddelegate.cpp
    protocols/fdo/x11embedpainter.cpp

    protocols/plasmoid/plasmoidtaskprotocol.cpp
    protocols/plasmoid/plasmoidtask.cpp

    protocols/notifications/dbusnotificationprotocol.cpp
    protocols/notifications/dbusnotification.cpp

    protocols/jobs/dbusjobprotocol.cpp
    protocols/jobs/dbusjob.cpp

    protocols/dbussystemtray/dbussystemtraytask.cpp
    protocols/dbussystemtray/dbussystemtrayprotocol.cpp
    protocols/dbussystemtray/dbussystemtraywidget.cpp
    protocols/dbussystemtray/systemtraytypes.cpp

    ui/applet.cpp
    ui/compactlayout.cpp
    ui/taskarea.cpp
    ui/notificationwidget.cpp
    ui/jobtotalswidget.cpp
    ui/jobwidget.cpp
    )

kde4_add_ui_files(systemtray_SRCS
                  ui/protocols.ui
                  ui/autohide.ui
                  ui/plasmoidtasks.ui)

add_subdirectory(notificationitemwatcher)

include (CheckLibraryExists)
check_library_exists (Xss XScreenSaverQueryInfo "" HAVE_LIBXSS)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
IF (HAVE_LIBXSS)
   SET (IDLE_DETECTION_LIB "Xss")
ENDIF (HAVE_LIBXSS)

set(notificationitemwatcher_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.NotificationItemWatcher.xml)
QT4_ADD_DBUS_INTERFACE(systemtray_SRCS ${notificationitemwatcher_xml} notificationitemwatcher_interface)


set(notificationitem_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.NotificationItem.xml)

set_source_files_properties(${notificationitem_xml} PROPERTIES
   NO_NAMESPACE false
   INCLUDE "protocols/dbussystemtray/systemtraytypes.h"
   CLASSNAME OrgKdeNotificationItem
)
QT4_ADD_DBUS_INTERFACE(systemtray_SRCS ${notificationitem_xml} notificationitem_interface)

kde4_add_plugin(plasma_applet_systemtray ${systemtray_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_systemtray ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} ${X11_Xrender_LIB} ${KDE4_SOLID_LIBS})
if(HAVE_XCOMPOSITE AND HAVE_XFIXES AND HAVE_XDAMAGE)
target_link_libraries(plasma_applet_systemtray ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${X11_Xcomposite_LIB})
endif(HAVE_XCOMPOSITE AND HAVE_XFIXES AND HAVE_XDAMAGE)
if(HAVE_LIBXSS)
   target_link_libraries(plasma_applet_systemtray ${IDLE_DETECTION_LIB})
endif(HAVE_LIBXSS)

install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})
