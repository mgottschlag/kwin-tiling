# Project Needs a name ofcourse
project(iondebianweather)
 
# We add our source code here
set(karaboudjan_SRCS iondebianweather.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(ion_debianweather ${karaboudjan_SRCS})
target_link_libraries(ion_debianweather weather_ion
                      ${QT_QTCORE_LIBRARY})
 
install(TARGETS ion_debianweather
       DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES ion-debianweather.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
