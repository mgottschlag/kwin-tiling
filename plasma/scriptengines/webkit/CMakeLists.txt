project(plasma-webapplet)

set(webapplet_SRCS
    webpage.cpp webapplet.cpp webapplet_plugin.cpp plasmajs.cpp)

include_directories( ${QT_INCLUDE_DIR}/QtWebKit )

kde4_add_plugin(plasma_appletscriptengine_webapplet ${webapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_webapplet plasma ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_webapplet DESTINATION ${PLUGIN_INSTALL_DIR})

set(dashboardapplet_SRCS
    webpage.cpp webapplet.cpp plasmajs.cpp dashboardapplet.cpp bundle.cpp)
kde4_add_plugin(plasma_appletscriptengine_dashboard ${dashboardapplet_SRCS})
target_link_libraries(plasma_appletscriptengine_dashboard plasma ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_appletscriptengine_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

set(bundlepackage_SRCS
    bundle.cpp
    dashboard_plugin.cpp)
kde4_add_plugin(plasma_packagestructure_dashboard ${bundlepackage_SRCS})
target_link_libraries(plasma_packagestructure_dashboard  plasma ${KDE4_KIO_LIBS} ${QT_QTWEBKIT_LIBRARY} )
install(TARGETS plasma_packagestructure_dashboard DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-scriptengine-applet-web.desktop
              plasma-scriptengine-applet-dashboard.desktop
              plasma-packagestructure-dashboard.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
