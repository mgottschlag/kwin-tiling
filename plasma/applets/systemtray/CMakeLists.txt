project(plasma-systemtray)

set(systemtray_SRCS

    core/manager.cpp
    core/taskprotocol.cpp
    core/task.cpp
    core/notificationprotocol.cpp
    core/notification.cpp

    protocols/fdo/fdotaskprotocol.cpp
    protocols/fdo/fdotask.cpp
    protocols/fdo/fdonotificationprotocol.cpp
    protocols/fdo/fdonotification.cpp
    protocols/fdo/fdographicswidget.cpp
    protocols/fdo/fdoselectionmanager.cpp
    protocols/fdo/x11embedcontainer.cpp
    protocols/fdo/x11embeddelegate.cpp

    protocols/plasmoid/plasmoidtaskprotocol.cpp
    protocols/plasmoid/plasmoidtask.cpp

    protocols/dbus/dbusnotificationprotocol.cpp
    protocols/dbus/dbusnotification.cpp

    ui/applet.cpp
    ui/compactlayout.cpp
    ui/taskarea.cpp
    ui/notificationwidget.cpp
    )

kde4_add_plugin(plasma_applet_systemtray ${systemtray_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_systemtray ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} ${X11_Xrender_LIB}
                      ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${X11_Xcomposite_LIB})

install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})
