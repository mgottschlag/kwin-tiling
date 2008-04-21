
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/libkhotkeys
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/libkhotkeys/actions
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/libkhotkeys/conditions
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/libkhotkeys/triggers
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/libkhotkeys )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

# Convenience library
add_subdirectory( libkhotkeys )
# kcm module to configure hotkeys
add_subdirectory( kcm_hotkeys )
# kdedmodule and standalone demon for hotkeys
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )
add_subdirectory( test )

########### install files ###############

install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )

