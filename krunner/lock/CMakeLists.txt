include_directories( ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcheckpass ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kdm )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

########### next target ###############

check_library_exists(Xxf86misc XF86MiscSetGrabKeysState "" HAVE_XF86MISCSETGRABKEYSSTATE)
check_library_exists(GL glXChooseVisual "" HAVE_GLXCHOOSEVISUAL)

configure_file(config-krunner-lock.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-krunner-lock.h)

set(krunner_lock_SRCS
   lockprocess.cc
   lockdlg.cc
   autologout.cc
   main.cc )

set(ksmserver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(krunner_lock_SRCS ${ksmserver_xml} ksmserver_interface)
set(kscreensaver_xml  ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.kde.screensaver.xml)
QT4_ADD_DBUS_INTERFACE(krunner_lock_SRCS ${kscreensaver_xml} kscreensaver_interface)

set(kxkb_adaptor ${KDEBASE_WORKSPACE_SOURCE_DIR}/kcontrol/kxkb/kxkb_adaptor.h)
set(kxkb_xml ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KXKB.xml)
ADD_CUSTOM_COMMAND(OUTPUT ${kxkb_xml}
       COMMAND ${QT_DBUSCPP2XML_EXECUTABLE} ${kxkb_adaptor} > ${kxkb_xml}
       DEPENDS ${kxkb_adaptor})
QT4_ADD_DBUS_INTERFACE( krunner_lock_SRCS ${kxkb_xml} kxkb_interface )



kde4_add_kcfg_files(krunner_lock_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/../kscreensaversettings.kcfgc)



kde4_add_executable(krunner_lock ${krunner_lock_SRCS} )

# needs qt3support for Q3ListView
target_link_libraries(krunner_lock  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KIO_LIBS} kworkspace ${QT_QT3SUPPORT_LIBRARY})

if (HAVE_XF86MISC)
  target_link_libraries(krunner_lock ${X11_Xxf86misc_LIB})
endif (HAVE_XF86MISC)
if(HAVE_GLXCHOOSEVISUAL)
   target_link_libraries(krunner_lock ${OPENGL_gl_LIBRARY})
endif(HAVE_GLXCHOOSEVISUAL)

install(TARGETS krunner_lock DESTINATION ${LIBEXEC_INSTALL_DIR})

install_pam_service(kscreensaver)


########### install files ###############



