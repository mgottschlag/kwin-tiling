set(apps_engine_SRCS
    appsengine.cpp
    appsource.cpp
    appservice.cpp
    appjob.cpp
)

kde4_add_plugin(plasma_engine_apps ${apps_engine_SRCS})
target_link_libraries(plasma_engine_apps ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS plasma_engine_apps DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-dataengine-apps.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES apps.operations DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
