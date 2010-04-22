project(plasma-systemtray)
#TODO: see if is still the case
# 'engineName' causes error
kde4_no_enable_final(plasma-systemtray)

set(systemtray_SRCS

    core/manager.cpp
    core/protocol.cpp
    core/task.cpp

    protocols/fdo/fdoprotocol.cpp
    protocols/fdo/fdotask.cpp
    #FIXME: we should find another way to achieve that
    #protocols/fdo/fdonotification.cpp
    protocols/fdo/fdographicswidget.cpp
    protocols/fdo/fdoselectionmanager.cpp
    protocols/fdo/x11embedcontainer.cpp
    protocols/fdo/x11embeddelegate.cpp
    protocols/fdo/x11embedpainter.cpp

    protocols/plasmoid/plasmoidtaskprotocol.cpp
    protocols/plasmoid/plasmoidtask.cpp

    protocols/dbussystemtray/dbussystemtraytask.cpp
    protocols/dbussystemtray/dbussystemtrayprotocol.cpp
    protocols/dbussystemtray/dbussystemtraywidget.cpp

    ui/applet.cpp
    ui/compactlayout.cpp
    ui/taskarea.cpp
    )

kde4_add_ui_files(systemtray_SRCS
                  ui/autohide.ui
                  ui/visibleitems.ui)

include (CheckLibraryExists)
check_library_exists (Xss XScreenSaverQueryInfo "" HAVE_LIBXSS)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
IF (HAVE_LIBXSS)
   SET (IDLE_DETECTION_LIB "Xss")
ENDIF (HAVE_LIBXSS)

kde4_add_plugin(plasma_applet_systemtray ${systemtray_SRCS})
include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(plasma_applet_systemtray ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} ${X11_Xrender_LIB} ${X11_Xfixes_LIB} ${X11_Xdamage_LIB} ${X11_Xcomposite_LIB} ${KDE4_SOLID_LIBS} ${IDLE_DETECTION_LIB})

install(TARGETS plasma_applet_systemtray DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-systemtray.desktop DESTINATION ${SERVICES_INSTALL_DIR})
