include_directories( ${CMAKE_SOURCE_DIR}/apps/lib/konq )

# Kickoff Shared
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
       ui/urlitemview.cpp  )

set ( Kickoff_LIBS
        ${KDE4_KIO_LIBS}
        ${KDE4_SOLID_LIBS}
        konq
        kworkspace
        strigiqtdbusclient )

# Standalone Test Application
IF (CMAKE_BUILD_TYPE MATCHES debug)
   set ( Application_SRCS
         ${Kickoff_SRCS}
         main.cpp )

   kde4_add_executable(kickoff ${Application_SRCS})
   target_link_libraries(kickoff ${Kickoff_LIBS})

   install(TARGETS kickoff DESTINATION ${BIN_INSTALL_DIR})
ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)

# Plasma Applet
set ( Applet_SRCS
        ${Kickoff_SRCS}
        applet/applet.cpp )

kde4_add_plugin(plasma_applet_launcher ${Applet_SRCS})
target_link_libraries(plasma_applet_launcher plasma ${Kickoff_LIBS}) 
install(TARGETS plasma_applet_launcher DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES applet/plasma-launcher-applet.desktop DESTINATION ${SERVICES_INSTALL_DIR})

