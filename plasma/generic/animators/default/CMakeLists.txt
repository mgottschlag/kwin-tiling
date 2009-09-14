set(default_animator_SRCS
    defaultAnimator.cpp
)

kde4_add_plugin(plasma_animator_default ${default_animator_SRCS})
target_link_libraries(plasma_animator_default ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_animator_default DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-animator-default.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

