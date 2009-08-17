project(plasma-contextaction-paste)

set(paste_SRCS
    paste.cpp
)

kde4_add_plugin(plasma_contextaction_paste ${paste_SRCS})
target_link_libraries(plasma_contextaction_paste ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_paste DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-paste.desktop DESTINATION ${SERVICES_INSTALL_DIR})
