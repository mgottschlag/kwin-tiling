
if(WITH_OpenGL)
check_library_exists(GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL)
endif(WITH_OpenGL)
configure_file (config-screensaver.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-screensaver.h )

########### next target ###############

set(kcm_screensaver_PART_SRCS
   scrnsave.cpp
   testwin.cpp
   saverconfig.cpp
   kswidget.cpp)

set(kscreensaver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.screensaver.xml)
QT4_ADD_DBUS_INTERFACE( kcm_screensaver_PART_SRCS ${kscreensaver_xml} kscreensaver_interface )



kde4_add_ui_files(kcm_screensaver_PART_SRCS screensaver.ui )

kde4_add_plugin(kcm_screensaver ${kcm_screensaver_PART_SRCS})


target_link_libraries(kcm_screensaver ${KDE4_PLASMA_LIBS} ${X11_LIBRARIES} kworkspace)

find_library(DL_LIBRARY dl)
if (DL_LIBRARY)
  target_link_libraries(kcm_screensaver ${DL_LIBRARY})
endif(DL_LIBRARY)

if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(kcm_screensaver ${OPENGL_gl_LIBRARY})
endif(HAVE_GLXCHOOSEVISUAL)


install(TARGETS kcm_screensaver  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES screensaver.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

