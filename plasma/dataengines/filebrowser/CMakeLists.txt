set(filebrowser_engine_SRCS
    filebrowserengine.cpp
)

kde4_add_plugin(plasma_engine_filebrowser ${filebrowser_engine_SRCS})
target_link_libraries(plasma_engine_filebrowser ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} plasma)

install(TARGETS plasma_engine_filebrowser DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-filebrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

