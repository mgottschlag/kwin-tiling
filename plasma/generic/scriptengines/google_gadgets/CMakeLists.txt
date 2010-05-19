project(plasma-ggl)

if(NOT KDE4_FOUND)
   find_package(KDE4 REQUIRED)
   find_package(Googlegadgets REQUIRED)

   include(KDE4Defaults)

   include_directories(
     ${KDE4_INCLUDES}
     )
endif(NOT KDE4_FOUND)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (${GOOGLEGADGETS_CFLAGS_OTHER})

include_directories(
   ${GOOGLEGADGETS_INCLUDE_DIRS}
   )

execute_process(COMMAND pkg-config --variable=ggl_resource_dir libggadget-1.0
    OUTPUT_VARIABLE gadgets_path)
string(REPLACE "\n" "" "gadgets_path" "${gadgets_path}")

# We add our source code here
set(package_SRCS ggl_package.cpp)
set(scriptengine_SRCS
  ggl_applet_script.cpp
  plasma_host.cpp
  plasma_view_host.cpp
  panel_decorator.cpp
  floating_decorator.cpp
  popout_decorator.cpp
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
install(FILES plasma-packagestructure-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS plasma_scriptengine_ggl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
