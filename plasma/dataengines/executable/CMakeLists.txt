set(executable_engine_SRCS
    executable.cpp
)

kde4_add_plugin(plasma_engine_executable ${executable_engine_SRCS})
target_link_libraries(plasma_engine_executable ${KDE4_KDECORE_LIBS} ${KDE4_SYNDICATION_LIBS} ${PLASMA_LIBS})

install(TARGETS plasma_engine_executable DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-executable.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

