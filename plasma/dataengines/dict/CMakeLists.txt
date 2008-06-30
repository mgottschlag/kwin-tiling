project(plasma-dictengine)

set(dict_engine_SRCS
    dictengine.cpp
)

kde4_add_plugin(plasma_engine_dict ${dict_engine_SRCS})
target_link_libraries(plasma_engine_dict ${KDE4_KDECORE_LIBS} plasma ${QT_QTNETWORK_LIBRARY})

install(TARGETS plasma_engine_dict DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-dict.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

