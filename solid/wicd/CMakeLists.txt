project(solid-wicd)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include(MacroOptionalAddSubdirectory)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BUILD_DIR}
)

#add_subdirectory(dbus)

#add_subdirectory(tests)

set(solid_wicd_PART_SRCS
   wirelessaccesspoint.cpp
   networkmanager.cpp
   networkinterface.cpp
   wirelessnetworkinterface.cpp
   wirednetworkinterface.cpp
)

kde4_add_library(solid_wicd_static STATIC ${solid_wicd_PART_SRCS})
target_link_libraries(solid_wicd_static ${KDE4_KDECORE_LIBS} solidcontrolifaces)

kde4_add_plugin(solid_wicd module.cpp ${solid_wicd_PART_SRCS})

target_link_libraries(solid_wicd ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} solidcontrol solidcontrolifaces)
install(TARGETS solid_wicd DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_wicd.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )

kde4_install_icons( ${ICON_INSTALL_DIR} )
