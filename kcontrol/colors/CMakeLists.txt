KDE4_NO_ENABLE_FINAL(colors)

set(kcm_colors_PART_SRCS colorscm.cpp previewwidget.cpp)

kde4_add_ui_files(kcm_colors_PART_SRCS colorsettings.ui preview.ui)

kde4_add_plugin(kcm_colors ${kcm_colors_PART_SRCS})
target_link_libraries(kcm_colors ${KDE4_KFILE_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS kcm_colors  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES colors.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

# built-in color schemes
set(schemefiles
    schemes/BeOS
    "schemes/Midnight Meadow"
)
install( FILES ${schemefiles} DESTINATION ${DATA_INSTALL_DIR}/color-schemes )

kde4_install_icons( ${ICON_INSTALL_DIR}  )
