project(plasma-systemtray)

set(systemtray_SRCS

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

    ui/applet.cpp
    ui/compactlayout.cpp
    ui/extendertask.cpp
    ui/taskarea.cpp
    ui/notificationwidget.cpp
    ui/jobwidget.cpp
    )

kde4_add_plugin(plasma_applet_systemtray ${systemtray_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_systemtray ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} ${X11_Xrender_LIB} ${KDE4_SOLID_LIBS})
if(HAVE_XCOMPOSITE AND HAVE_XFIXES AND HAVE_XDAMAGE)
target_link_libraries(plasma_applet_systemtray ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${X11_Xcomposite_LIB})
endif(HAVE_XCOMPOSITE AND HAVE_XFIXES AND HAVE_XDAMAGE)

install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})
