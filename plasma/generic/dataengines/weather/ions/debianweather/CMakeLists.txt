# Project Needs a name ofcourse
project(iondebianweather)
 
# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
find_package(KDE4Workspace REQUIRED)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
   )
 
# We add our source code here
set(karaboudjan_SRCS iondebianweather.cpp)
 
# Now make sure all files get to the right place
kde4_add_plugin(ion_debianweather ${karaboudjan_SRCS})
target_link_libraries(ion_debianweather ${KDE4WORKSPACE_WEATHERION_LIBRARY}
                      ${QT_QTCORE_LIBRARY})
 
install(TARGETS ion_debianweather
       DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES ion-debianweather.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
