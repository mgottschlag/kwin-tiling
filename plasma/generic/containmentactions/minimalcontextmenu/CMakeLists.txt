project(plasma-containmentactions-minimalcontextmenu)

set(contextmenu_SRCS
    menu.cpp
)

kde4_add_plugin(plasma_containmentactions_minimalcontextmenu ${contextmenu_SRCS})
if(WIN32)
target_link_libraries(plasma_containmentactions_minimalcontextmenu ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})
else(WIN32)
target_link_libraries(plasma_containmentactions_minimalcontextmenu ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})
endif(WIN32)

install(TARGETS plasma_containmentactions_minimalcontextmenu DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-minimalcontextmenu.desktop DESTINATION ${SERVICES_INSTALL_DIR})
