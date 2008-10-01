set(rss_engine_SRCS
    rss.cpp
)

kde4_add_plugin(plasma_engine_rss ${rss_engine_SRCS})
target_link_libraries(plasma_engine_rss ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} ${PLASMA_LIBS})

install(TARGETS plasma_engine_rss DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-rss.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

