#######################################
# Effect

# Source files
set( kwin4_effect_builtins_sources ${kwin4_effect_builtins_sources}
    login/login.cpp
    )

# .desktop files
install( FILES
    login/login.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

#######################################
# Config

# Source files
set( kwin4_effect_builtins_config_sources ${kwin4_effect_builtins_config_sources}
    login/login_config.cpp
    login/login_config.ui
    )

# .desktop files
install( FILES
    login/login_config.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}/kwin )