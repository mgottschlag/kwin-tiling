PROJECT (autostart)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE (KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

########### next target ###############

set(kcm_autostart_PART_SRCS
   adddialog.cpp
   listdelegate.cpp
   autostart.cpp )

kde4_add_ui_files(kcm_autostart_PART_SRCS autostartconfig.ui addDialog.ui )

kde4_add_plugin(kcm_autostart ${kcm_autostart_PART_SRCS})

target_link_libraries(kcm_autostart   ${KDE4_KIO_LIBS} )

install(TARGETS kcm_autostart  DESTINATION ${PLUGIN_INSTALL_DIR} )

########### install files ###############

install( FILES autostart.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}   )
