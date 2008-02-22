# Kickoff Shared
if(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)
   set(HAVE_STRIGIDBUS 1)
endif(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)
configure_file(core/config-kickoff-applets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kickoff-applets.h )

set( Kickoff_SRCS
       core/applicationmodel.cpp
       core/favoritesmodel.cpp
       core/leavemodel.cpp
       core/models.cpp
       core/recentapplications.cpp
       core/recentlyusedmodel.cpp
       core/searchmodel.cpp
       core/systemmodel.cpp
       core/urlitemlauncher.cpp
       core/itemhandlers.cpp
       ui/itemdelegate.cpp
       ui/contextmenufactory.cpp
       ui/flipscrollview.cpp
       ui/launcher.cpp
       ui/searchbar.cpp
       ui/tabbar.cpp
       ui/urlitemview.cpp )

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(Kickoff_SRCS ${screensaver_xml} screensaver_interface)
set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/org.kde.krunner.Interface.xml)
QT4_ADD_DBUS_INTERFACE(Kickoff_SRCS ${krunner_xml} krunner_interface)

set ( Kickoff_LIBS
        ${KDE4_KIO_LIBS}
        ${KDE4_KFILE_LIBS}
        ${KDE4_SOLID_LIBS}
        kworkspace)
if(HAVE_STRIGIDBUS)
     set ( Kickoff_LIBS ${Kickoff_LIBS} strigiqtdbusclient )
endif(HAVE_STRIGIDBUS)

include_directories( ${STRIGI_INCLUDE_DIR} )

# Standalone Test Application
IF (CMAKE_BUILD_TYPE MATCHES Debug)
   set ( Application_SRCS
         ${Kickoff_SRCS}
         main.cpp )

   kde4_add_executable(kickoff ${Application_SRCS})
   target_link_libraries(kickoff plasma ${Kickoff_LIBS})

   install(TARGETS kickoff DESTINATION ${BIN_INSTALL_DIR})
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

# Kickoff Plasma Applet
set ( Applet_SRCS ${Kickoff_SRCS} applet/applet.cpp )
kde4_add_plugin(plasma_applet_launcher ${Applet_SRCS})
target_link_libraries(plasma_applet_launcher plasma ${Kickoff_LIBS}) 
install(TARGETS plasma_applet_launcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES applet/plasma-applet-launcher.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# Simple KMenu Plasma Applet
set ( SimpleApplet_SRCS ${Kickoff_SRCS} simpleapplet/menuview.cpp simpleapplet/simpleapplet.cpp )
kde4_add_plugin(plasma_applet_simplelauncher ${SimpleApplet_SRCS})
target_link_libraries(plasma_applet_simplelauncher plasma ${Kickoff_LIBS}) 
install(TARGETS plasma_applet_simplelauncher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES simpleapplet/plasma-applet-simplelauncher.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES ui/kickoff-branding.png DESTINATION ${DATA_INSTALL_DIR}/desktoptheme/default/)
