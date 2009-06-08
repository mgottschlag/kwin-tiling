project(plasma-searchbox)


find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )


set(searchbox_SRCS searchbox.cpp)


kde4_add_plugin(plasma_applet_searchbox ${searchbox_SRCS})
target_link_libraries(plasma_applet_searchbox 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_searchbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-searchbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
