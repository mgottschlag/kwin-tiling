project(quicklaunch-applet)

# building separately or as part of kdebase ?
if(NOT KDE4_FOUND)
   find_package(KDE4 REQUIRED)

   include_directories(
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES} 
   )
endif(NOT KDE4_FOUND)

set(plasma_applet_quicklaunch_SRCS
  quicklaunchApplet.cpp
  quicklaunchIcon.cpp
)

kde4_add_ui_files(plasma_applet_quicklaunch_SRCS quicklaunchConfig.ui)
kde4_add_ui_files(plasma_applet_quicklaunch_SRCS quicklaunchAdd.ui)
kde4_add_plugin(plasma_applet_quicklaunch ${plasma_applet_quicklaunch_SRCS})
target_link_libraries(plasma_applet_quicklaunch ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBRARY} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_applet_quicklaunch DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-quicklaunch.desktop DESTINATION ${SERVICES_INSTALL_DIR})
