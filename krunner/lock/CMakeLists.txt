include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm )

########### next target ###############

check_library_exists(Xxf86misc XF86MiscSetGrabKeysState "" HAVE_XF86MISCSETGRABKEYSSTATE)
if(WITH_OpenGL)
check_library_exists(GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL)
endif(WITH_OpenGL)

configure_file(config-krunner-lock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krunner-lock.h)

set(kscreenlocker_SRCS
   lockprocess.cc
   lockdlg.cc
   autologout.cc
   main.cc )

set(plasmaapp_dbusXML ../../plasma/shells/screensaver/org.kde.plasma-overlay.App.xml)
qt4_add_dbus_interface(kscreenlocker_SRCS ${plasmaapp_dbusXML} plasmaapp_interface)

set(lockprocess_dbusXML org.kde.screenlocker.LockProcess.xml)
qt4_generate_dbus_interface(lockprocess.h ${lockprocess_dbusXML} OPTIONS -m)
qt4_add_dbus_adaptor(kscreenlocker_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${lockprocess_dbusXML} lockprocess.h LockProcess)

set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(kscreenlocker_SRCS ${ksmserver_xml} ksmserver_interface)
set(kscreensaver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.screensaver.xml)
QT4_ADD_DBUS_INTERFACE(kscreenlocker_SRCS ${kscreensaver_xml} kscreensaver_interface)

set(kxkb_adaptor ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kxkb/kxkb_adaptor.h)
set(kxkb_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KXKB.xml)
ADD_CUSTOM_COMMAND(OUTPUT ${kxkb_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kxkb_adaptor} > ${kxkb_xml}
       DEPENDS ${kxkb_adaptor})
QT4_ADD_DBUS_INTERFACE( kscreenlocker_SRCS ${kxkb_xml} kxkb_interface )



kde4_add_kcfg_files(kscreenlocker_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kcfg/kscreensaversettings.kcfgc)



kde4_add_executable(kscreenlocker ${kscreenlocker_SRCS})

target_link_libraries(kscreenlocker  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})

find_library(DL_LIBRARY dl)
if (DL_LIBRARY)
  target_link_libraries(kscreenlocker ${DL_LIBRARY})
endif(DL_LIBRARY)

target_link_libraries(kscreenlocker kworkspace)

if (HAVE_XF86MISC)
  target_link_libraries(kscreenlocker ${X11_Xxf86misc_LIB})
endif (HAVE_XF86MISC)

if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(kscreenlocker ${OPENGL_gl_LIBRARY})
endif(HAVE_GLXCHOOSEVISUAL)

install(TARGETS kscreenlocker DESTINATION ${LIBEXEC_INSTALL_DIR})

install_pam_service(kscreensaver)


########### install files ###############



