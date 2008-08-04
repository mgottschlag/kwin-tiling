project(plasma-ggl)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} "-D_DEBUG -D__STDC_CONSTANT_MACROS")

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${QT_INCLUDES}
   ${GGLQT_INCLUDE_DIRS}
   ${KDE4_INCLUDES}
   ${GOOGLEGADGETS_INCLUDE_DIR}
   )

STRING(REPLACE "-Wall" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
STRING(REPLACE "-Wall" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
STRING(REPLACE "-Werror" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
STRING(REPLACE "-Werror" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# We add our source code here
set(package_SRCS ggl_package.cpp)
set(scriptengine_SRCS ggl_applet_script.cpp plasma_host.cpp plasma_view_host.cpp)

# Now make sure all files get to the right place
kde4_add_plugin(plasma_package_ggl ${package_SRCS})
target_link_libraries(plasma_package_ggl
  ${GOOGLEGADGETS_LIBRARIES}
  ${PLASMA_LIBS} ${KDE4_KDECORE_LIBS} )

kde4_add_plugin(plasma_scriptengine_ggl ${scriptengine_SRCS})
target_link_libraries(plasma_scriptengine_ggl
  ${GOOGLEGADGETS_LIBRARIES} ${GOOGLEGADGETSQT_LIBRARIES}
  ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS} )

install(TARGETS plasma_package_ggl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-package-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS plasma_scriptengine_ggl DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-googlegadgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
