# Project Needs a name ofcourse
project(plasma-activitybar)
include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)
 
# We add our source code here
set(activitybar_SRCS activitybar.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_activitybar ${activitybar_SRCS})
target_link_libraries(plasma_applet_activitybar 
                      plasma ${KDE4_KDEUI_LIBS} kworkspace)
 
install(TARGETS plasma_applet_activitybar
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-activitybar.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
