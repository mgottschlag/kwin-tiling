
include_directories(${Boost_INCLUDE_DIR} ${KDEPIMLIBS_INCLUDE_DIR})

set(rss_engine_SRCS
    rss.cpp
)

kde4_add_plugin(plasma_engine_rss ${rss_engine_SRCS})
target_link_libraries(plasma_engine_rss ${KDE4_PLASMA_LIBS} ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_engine_rss DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-rss.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

