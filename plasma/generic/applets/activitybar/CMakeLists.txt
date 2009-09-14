# Project Needs a name ofcourse
project(plasma-activitybar)
 
# We add our source code here
set(activitybar_SRCS activitybar.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(plasma_applet_activitybar ${activitybar_SRCS})
target_link_libraries(plasma_applet_activitybar 
                      plasma ${KDE4_KDEUI_LIBS})
 
install(TARGETS plasma_applet_activitybar
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-applet-activitybar.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
