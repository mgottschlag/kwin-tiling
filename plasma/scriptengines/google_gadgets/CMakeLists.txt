project(plasma-ggl)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions (${GOOGLEGADGETS_CFLAGS_OTHER})

include_directories(
   ${GOOGLEGADGETS_INCLUDE_DIRS}
   )

execute_process(COMMAND pkg-config --variable=ggl_resource_dir libggadget-1.0
    OUTPUT_VARIABLE gadgets_path)
string(REPLACE "\n" "" "gadgets_path" "${gadgets_path}")

macro(install_ggl_gadget _name)
  find_file(${_name}_path ${_name}.gg ${gadgets_path})
  if (NOT ${${_name}_path} MATCHES "NOTFOUND")
    set(gadget_name "ggl_${_name}")
    set(gadget_readable_name ${_name})
    set(gadget_comment ${_name})
    set(gadget_path "${${_name}_path}")
    set(gadget_options_name "${_name}_builtin")
    configure_file(plasma-applet-ggl.desktop.tmpl
      ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-ggl-${_name}.desktop
      @only)
    configure_file(config.txt.tmpl
      ${CMAKE_CURRENT_BINARY_DIR}/config.txt.${_name}
      @only)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plasma-applet-ggl-${_name}.desktop
      DESTINATION ${SERVICES_INSTALL_DIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.txt.${_name}
      DESTINATION ${DATA_INSTALL_DIR}/plasma/plasmoids/${gadget_name}
      RENAME config.txt)
  endif (NOT ${${_name}_path} MATCHES "NOTFOUND")
endmacro(install_ggl_gadget _name)

# We add our source code here
set(package_SRCS ggl_package.cpp)
set(scriptengine_SRCS
  ggl_applet_script.cpp
  plasma_host.cpp
  plasma_view_host.cpp
  panel_decorator.cpp
  floating_decorator.cpp
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

install_ggl_gadget(photos)
install_ggl_gadget(rss)
install_ggl_gadget(analog_clock)
install_ggl_gadget(weather_globe)
