#######################################################################################
# Kickoff Library

if(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)
    set(HAVE_STRIGIDBUS 1)
endif(STRIGI_STRIGIQTDBUSCLIENT_LIBRARY)

configure_file(core/config-kickoff-applets.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kickoff-applets.h)

set(libkickoff_SRCS
    core/kickoffmodel.cpp
    core/kickoffabstractmodel.cpp
    core/kickoffproxymodel.cpp
    core/applicationmodel.cpp
    core/favoritesmodel.cpp
    core/leavemodel.cpp
    core/models.cpp
    core/recentapplications.cpp
    core/recentlyusedmodel.cpp
    core/krunnermodel.cpp
    core/systemmodel.cpp
    core/urlitemlauncher.cpp
    core/itemhandlers.cpp
)

qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.xml core/applicationmodel.h Kickoff::ApplicationModel)
qt4_add_dbus_adaptor(libkickoff_SRCS core/org.kde.kickoff.recent.xml core/recentlyusedmodel.h Kickoff::RecentlyUsedModel)

set(screensaver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.freedesktop.ScreenSaver.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${screensaver_xml} screensaver_interface)
set(krunner_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/krunner/dbus/org.kde.krunner.App.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${krunner_xml} krunner_interface)
set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
QT4_ADD_DBUS_INTERFACE(libkickoff_SRCS ${ksmserver_xml} ksmserver_interface)

set(Kickoff_LIBS ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_SOLID_LIBS} kworkspace)
if(HAVE_STRIGIDBUS)
     set ( Kickoff_LIBS ${Kickoff_LIBS} ${STRIGI_STRIGIQTDBUSCLIENT_LIBRARY} )
endif(HAVE_STRIGIDBUS)

include_directories( ${STRIGI_INCLUDE_DIR} )

kde4_add_library(kickoff SHARED ${libkickoff_SRCS})
target_link_libraries(kickoff ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS})
install(TARGETS kickoff ${INSTALL_TARGETS_DEFAULT_ARGS})

#######################################################################################
# Kickoff Plasma Applet

set(Applet_SRCS
    ui/brandingbutton.cpp
    ui/contextmenufactory.cpp
    ui/flipscrollview.cpp
    ui/itemdelegate.cpp
    ui/contentareacap.cpp
    ui/launcher.cpp
    ui/searchbar.cpp
    ui/tabbar.cpp
    ui/urlitemview.cpp
    applet/applet.cpp
)
kde4_add_ui_files(Applet_SRCS applet/kickoffConfig.ui )
kde4_add_plugin(plasma_applet_launcher ${Applet_SRCS})
target_link_libraries(plasma_applet_launcher ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS} kickoff)
install(TARGETS plasma_applet_launcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES applet/plasma-applet-launcher.desktop DESTINATION ${SERVICES_INSTALL_DIR})

#######################################################################################
# Kickoff Simple KMenu Plasma Applet

set(SimpleApplet_SRCS
    ui/contextmenufactory.cpp
    simpleapplet/menuview.cpp
    simpleapplet/simpleapplet.cpp
)
kde4_add_plugin(plasma_applet_simplelauncher ${SimpleApplet_SRCS})
target_link_libraries(plasma_applet_simplelauncher ${KDE4_KCMUTILS_LIBS} ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS} kickoff)
install(TARGETS plasma_applet_simplelauncher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES simpleapplet/plasma-applet-simplelauncher.desktop DESTINATION ${SERVICES_INSTALL_DIR})

# Kickoff Standalone Test Application
#IF (CMAKE_BUILD_TYPE MATCHES Debug)
#    set(Application_SRCS ${Kickoff_SRCS} main.cpp)
#    kde4_add_executable(kickoff ${Application_SRCS})
#    target_link_libraries(kickoff ${KDE4_PLASMA_LIBS} ${Kickoff_LIBS})
#    install(TARGETS kickoff ${INSTALL_TARGETS_DEFAULT_ARGS})
#ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
