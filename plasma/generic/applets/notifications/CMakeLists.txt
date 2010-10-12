project(plasma-notifications)
#TODO: see if is still the case
# 'engineName' causes error
kde4_no_enable_final(plasma-notifications)

set(notifications_SRCS

    core/notificationsmanager.cpp
    core/protocol.cpp
    core/notification.cpp
    core/completedjobnotification.cpp
    core/job.cpp

    protocols/notifications/dbusnotificationprotocol.cpp
    protocols/notifications/dbusnotification.cpp

    protocols/jobs/dbusjobprotocol.cpp
    protocols/jobs/dbusjob.cpp

    ui/busywidget.cpp
    ui/notifications.cpp
    ui/notificationwidget.cpp
    ui/jobtotalswidget.cpp
    ui/jobwidget.cpp
    ui/notificationgroup.cpp
    ui/notificationstack.cpp
    ui/stackdialog.cpp
    )

kde4_add_ui_files(notifications_SRCS
                  ui/notificationsconfig.ui)


include (CheckLibraryExists)
check_library_exists (Xss XScreenSaverQueryInfo "" HAVE_LIBXSS)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config-notifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-notifications.h)
IF (HAVE_LIBXSS)
   SET (IDLE_DETECTION_LIB "Xss")
ENDIF (HAVE_LIBXSS)


kde4_add_plugin(plasma_applet_notifications ${notifications_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_notifications ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} ${X11_Xrender_LIB} ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${X11_Xcomposite_LIB} ${KDE4_SOLID_LIBS} ${IDLE_DETECTION_LIB})

install(TARGETS plasma_applet_notifications DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-notifications.desktop DESTINATION ${SERVICES_INSTALL_DIR})
