PROJECT (solid-actions)

find_package(KDE4 REQUIRED)
INCLUDE (KDE4Defaults)

ADD_SUBDIRECTORY(device-actions)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

########### next target ###############

SET(kcm_solid_actions_srcs
    action-item.cpp
    solid-action-data.cpp
    solid-action-edit-predicate.cpp
    solid-action-edit.cpp
    solid-actions.cpp )

SET(solid_action_desktop_gen_srcs
    solid-action-desktop-gen.cpp
    solid-action-data.cpp )

KDE4_ADD_UI_FILES(kcm_solid_actions_srcs
    solid-actions.ui
    solid-action-add.ui
    solid-action-edit.ui
    solid-action-edit-predicate.ui )

KDE4_ADD_PLUGIN(kcm_solid_actions ${kcm_solid_actions_srcs})
KDE4_ADD_EXECUTABLE(solid-action-desktop-gen ${solid_action_desktop_gen_srcs})

TARGET_LINK_LIBRARIES(kcm_solid_actions ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} )
TARGET_LINK_LIBRARIES(solid-action-desktop-gen ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} )

########### install files ###############

INSTALL( TARGETS kcm_solid_actions DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS solid-action-desktop-gen DESTINATION ${BIN_INSTALL_DIR} )
INSTALL( FILES solid-actions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
INSTALL( FILES solid-action-template.desktop DESTINATION ${DATA_INSTALL_DIR}/kcmsolidactions )

