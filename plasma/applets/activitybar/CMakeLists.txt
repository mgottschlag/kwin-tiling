# Project Needs a name ofcourse
project(plasma-activitybar)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(Plasma REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(activitybar_SRCS activitybar.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_activitybar ${activitybar_SRCS})
target_link_libraries(plasma_applet_activitybar 
                      ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_activitybar
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-activitybar.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
