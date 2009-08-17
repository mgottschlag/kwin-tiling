project(plasma-contextaction-test)

set(test_SRCS
    test.cpp
)
kde4_add_ui_files(test_SRCS config.ui)

kde4_add_plugin(plasma_contextaction_test ${test_SRCS})
target_link_libraries(plasma_contextaction_test ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_contextaction_test DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-contextaction-test.desktop DESTINATION ${SERVICES_INSTALL_DIR})
