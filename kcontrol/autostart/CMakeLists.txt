PROJECT (autostart)

FIND_PACKAGE(KDE4 REQUIRED)
INCLUDE (KDE4Defaults)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

########### next target ###############

SET(kcm_autostart_PART_SRCS
   autostartitem.cpp
   addscriptdialog.cpp
   autostart.cpp )

KDE4_ADD_UI_FILES(kcm_autostart_PART_SRCS autostartconfig.ui )

KDE4_ADD_PLUGIN(kcm_autostart ${kcm_autostart_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_autostart   ${KDE4_KIO_LIBS} )

########### install files ###############

INSTALL(TARGETS kcm_autostart  DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( FILES autostart.desktop   DESTINATION  ${SERVICES_INSTALL_DIR} )
