#######################################
# Effect

# Source files
set( kwin4_effect_builtins_sources ${kwin4_effect_builtins_sources}
     blur/blur.cpp
     blur/blurshader.cpp )

# .desktop files
install( FILES
         blur/blur.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

#######################################
# Config

# Source files
set( kwin4_effect_builtins_config_sources ${kwin4_effect_builtins_config_sources}
     blur/blur_config.cpp
     blur/blur_config.ui )

# .desktop files
install( FILES
         blur/blur_config.desktop
         DESTINATION ${SERVICES_INSTALL_DIR}/kwin )

