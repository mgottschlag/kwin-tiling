project(plasma-containmentactions-paste)

set(paste_SRCS
    paste.cpp
)

kde4_add_plugin(plasma_containmentactions_paste ${paste_SRCS})
target_link_libraries(plasma_containmentactions_paste ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containmentactions_paste DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containmentactions-paste.desktop DESTINATION ${SERVICES_INSTALL_DIR})
