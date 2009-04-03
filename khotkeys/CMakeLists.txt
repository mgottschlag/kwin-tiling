
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeysprivate
    ${CMAKE_CURRENT_BINARY_DIR}/libkhotkeysprivate
    )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1217)

# Convenience library
add_subdirectory( libkhotkeysprivate )
# has to be before kcm_hotkeys
add_subdirectory( app )
# kcm module to configure hotkeys
add_subdirectory( kcm_hotkeys )
# kdedmodule and standalone demon for hotkeys
add_subdirectory( data )
# add_subdirectory( test )

