
set(sal_SRCS
    sal.cpp
    griditemview.cpp
    stripwidget.cpp)

kde4_add_plugin(plasma_containment_sal ${sal_SRCS})
target_link_libraries(plasma_containment_sal ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_containment_sal DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-containment-sal.desktop DESTINATION ${SERVICES_INSTALL_DIR})
