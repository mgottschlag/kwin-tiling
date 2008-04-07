
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/libkhotkeysprivate
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate/actions
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate/conditions
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate/triggers
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

# Convenience library
add_subdirectory( libkhotkeysprivate )
# kcm module to configure hotkeys
add_subdirectory( kcontrol )
# kdedmodule and standalone demon for hotkeys
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )

########### install files ###############

install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )

