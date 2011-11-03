PROJECT (solid-actions)

# We allow this to be built seperately for convienence purposes
IF(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

    FIND_PACKAGE(KDE4 REQUIRED)
    INCLUDE(KDE4Defaults)
    ADD_DEFINITIONS(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

ENDIF(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

INCLUDE_DIRECTORIES (${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

ADD_SUBDIRECTORY(device-actions)

########### next target ###############

SET(kcm_solid_actions_srcs
    PredicateItem.cpp
    PredicateModel.cpp
    ActionItem.cpp
    ActionModel.cpp
    ActionEditor.cpp
    SolidActionData.cpp
    SolidActions.cpp )

SET(solid_action_desktop_gen_srcs
    DesktopFileGenerator.cpp
    SolidActionData.cpp )

KDE4_ADD_UI_FILES(kcm_solid_actions_srcs
    SolidActions.ui
    AddAction.ui
    ActionEditor.ui )

KDE4_ADD_PLUGIN(kcm_solid_actions ${kcm_solid_actions_srcs})
KDE4_ADD_EXECUTABLE(solid-action-desktop-gen ${solid_action_desktop_gen_srcs})

TARGET_LINK_LIBRARIES(kcm_solid_actions ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS} )
TARGET_LINK_LIBRARIES(solid-action-desktop-gen ${KDE4_SOLID_LIBS} ${KDE4_KIO_LIBS} )

########### install files ###############

INSTALL( TARGETS kcm_solid_actions DESTINATION ${PLUGIN_INSTALL_DIR} )
INSTALL( TARGETS solid-action-desktop-gen ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL( FILES solid-actions.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
INSTALL( FILES solid-action-template.desktop DESTINATION ${DATA_INSTALL_DIR}/kcmsolidactions )
INSTALL( FILES solid-device-type.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR} )

