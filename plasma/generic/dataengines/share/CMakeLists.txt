project(shareengine)

set(share_engine_SRCS
        shareprovider.cpp
        shareengine.cpp
        shareservice.cpp
        share_package.cpp)

kde4_add_plugin(plasma_engine_share ${share_engine_SRCS})
target_link_libraries(plasma_engine_share
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
                      ${KDE4_KIO_LIBS}
                      ${KDE4_KROSSCORE_LIBS})

install(TARGETS plasma_engine_share
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES data/plasma-dataengine-share.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/plasma_shareprovider.desktop
        DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(FILES share.operations
        DESTINATION ${DATA_INSTALL_DIR}/plasma/services)

set(sharepackage_SRCS
        share_package.cpp
        plugin_share_package.cpp
)

add_subdirectory(backends)

kde4_add_plugin(plasma_packagestructure_share
        ${sharepackage_SRCS})
target_link_libraries(plasma_packagestructure_share
        ${KDE4_PLASMA_LIBS})

install(TARGETS plasma_packagestructure_share
                 DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES data/plasma-packagestructure-share.desktop
               DESTINATION ${SERVICES_INSTALL_DIR})
