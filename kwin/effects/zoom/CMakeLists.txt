#######################################
# Effect

# Source files
set( kwin4_effect_builtins_sources ${kwin4_effect_builtins_sources}
    zoom/zoom.cpp
    )

# .desktop files
install( FILES
    zoom/zoom.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

#######################################
# Config

# Source files
set( kwin4_effect_builtins_config_sources ${kwin4_effect_builtins_config_sources}
    zoom/zoom_config.cpp
    zoom/zoom_config.ui
    )

# .desktop files
install( FILES
    zoom/zoom_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kwin )
