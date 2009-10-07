
set(newspaper_SRCS
    newspaper.cpp
    appletoverlay.cpp
    ../common/appletmovespacer.cpp
    ../common/nettoolbox.cpp
    applettitlebar.cpp)

kde4_add_plugin(plasma_containment_newspaper ${newspaper_SRCS})
target_link_libraries(plasma_containment_newspaper ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_newspaper DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-newspaper.desktop DESTINATION ${SERVICES_INSTALL_DIR})
