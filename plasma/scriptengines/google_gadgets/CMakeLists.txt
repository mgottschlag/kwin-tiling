project(plasma-ggl)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (${GOOGLEGADGETS_CFLAGS_OTHER})

include_directories(
   ${GOOGLEGADGETS_INCLUDE_DIRS}
   )

# We add our source code here
set(package_SRCS ggl_package.cpp)
set(scriptengine_SRCS
  ggl_applet_script.cpp
  plasma_host.cpp
  plasma_view_host.cpp
  panel_decorator.cpp
  )

# Now make sure all files get to the right place
kde4_add_plugin(plasma_package_ggl ${package_SRCS})
target_link_libraries(plasma_package_ggl
  ${GOOGLEGADGETS_LIBRARIES}
  plasma ${KDE4_KDECORE_LIBS} )

kde4_add_plugin(plasma_scriptengine_ggl ${scriptengine_SRCS})
target_link_libraries(plasma_scriptengine_ggl
  ${GOOGLEGADGETS_LIBRARIES}
  plasma ${KDE4_KDEUI_LIBS} )

install(TARGETS plasma_package_ggl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-package-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS plasma_scriptengine_ggl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
