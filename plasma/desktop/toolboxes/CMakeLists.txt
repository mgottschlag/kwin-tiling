project(plasma-toolbox)

#Desktop toolbox
set(desktoptoolbox_SRCS internaltoolbox.cpp desktoptoolbox.cpp)


kde4_add_plugin(plasma_toolbox_desktoptoolbox ${desktoptoolbox_SRCS})
target_link_libraries(plasma_toolbox_desktoptoolbox ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} kworkspace)

install(TARGETS plasma_toolbox_desktoptoolbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-toolbox-desktoptoolbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})


set(desktoptoolbox_SRCS internaltoolbox.cpp desktoptoolbox.cpp)


#Panel toolbox
set(paneltoolbox_SRCS internaltoolbox.cpp paneltoolbox.cpp)

kde4_add_plugin(plasma_toolbox_paneltoolbox ${paneltoolbox_SRCS})
target_link_libraries(plasma_toolbox_paneltoolbox ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

install(TARGETS plasma_toolbox_paneltoolbox
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-toolbox-paneltoolbox.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
