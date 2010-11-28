INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
)

set(krunner_powerdevil_SRCS
    PowerDevilRunner.cpp
)

kde4_add_plugin(krunner_powerdevil ${krunner_powerdevil_SRCS})
target_link_libraries(krunner_powerdevil ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS krunner_powerdevil DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-runner-powerdevil.desktop DESTINATION ${SERVICES_INSTALL_DIR})

