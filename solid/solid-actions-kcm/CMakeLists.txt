PROJECT (solid-actions)

find_package(KDE4 REQUIRED)
INCLUDE (KDE4Defaults)

ADD_DEFINITIONS (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
INCLUDE_DIRECTORIES (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

########### next target ###############

SET(kcm_solid_actions_srcs
    action-item.cpp
    solid-actions-edit.cpp
    solid-actions.cpp )

KDE4_ADD_UI_FILES(kcm_solid_actions_srcs solid-actions-config.ui solid-actions-edit.ui solid-actions-add.ui solid-actions-edit-predicate.ui )

KDE4_ADD_PLUGIN(kcm_solid_actions ${kcm_solid_actions_srcs})

TARGET_LINK_LIBRARIES(kcm_solid_actions ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} )

########### install files ###############

INSTALL(TARGETS kcm_solid_actions DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( FILES solid-actions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
