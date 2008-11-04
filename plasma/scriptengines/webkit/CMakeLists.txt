add_subdirectory(dashboard)

project(plasma-webapplet)

set(webapplet_SRCS
    webpage.cpp webapplet.cpp plasmawebapplet.cpp plasmajs.cpp webapplet_plugin.cpp)
kde4_add_plugin(plasma_appletscriptengine_webapplet ${webapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_webapplet ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_webapplet DESTINATION ${PLUGIN_INSTALL_DIR})

set(webappletpackage_SRCS
    webapplet_package.cpp)
kde4_add_plugin(plasma_packagestructure_web ${webappletpackage_SRCS})
target_link_libraries(plasma_packagestructure_web ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_packagestructure_web DESTINATION ${PLUGIN_INSTALL_DIR})

set(dashboardapplet_SRCS
    webpage.cpp webapplet.cpp dashboardapplet.cpp bundle.cpp dashboardjs.cpp)
kde4_add_plugin(plasma_appletscriptengine_dashboard ${dashboardapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_dashboard ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

set(bundlepackage_SRCS
    bundle.cpp dashboard_plugin.cpp)
kde4_add_plugin(plasma_packagestructure_dashboard ${bundlepackage_SRCS})
target_link_libraries(plasma_packagestructure_dashboard  ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_packagestructure_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-scriptengine-applet-web.desktop
              plasma-scriptengine-applet-dashboard.desktop
              plasma-packagestructure-dashboard.desktop
              plasma-packagestructure-web.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
