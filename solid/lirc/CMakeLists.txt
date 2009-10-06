include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BUILD_DIR}
)

set(solid_lirc_PART_SRCS
    lircclient.cpp
    lircremotecontrol.cpp
    lircremotecontrolmanager.cpp
)

kde4_add_library(solid_lirc_static STATIC ${solid_lirc_PART_SRCS})
target_link_libraries(solid_lirc_static ${KDE4_KDECORE_LIBS} solidcontrolifaces)

kde4_add_plugin(solid_lirc module.cpp ${solid_lirc_PART_SRCS})

target_link_libraries(solid_lirc ${KDE4_KDECORE_LIBS} ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} solidcontrol solidcontrolifaces)
install(TARGETS solid_lirc DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES solid_lirc.desktop DESTINATION ${SERVICES_INSTALL_DIR}/solidbackends )
