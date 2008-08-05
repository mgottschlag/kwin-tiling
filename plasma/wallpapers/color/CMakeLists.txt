set(color_SRCS
    color.cpp
)

kde4_add_plugin(plasma_wallpaper_color ${color_SRCS})
target_link_libraries(plasma_wallpaper_color plasma ${KDE4_KIO_LIBS})

install(TARGETS plasma_wallpaper_color DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-color.desktop DESTINATION ${SERVICES_INSTALL_DIR})
