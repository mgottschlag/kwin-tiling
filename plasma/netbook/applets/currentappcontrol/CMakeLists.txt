project(plasma-currentappcontrol)


set(currentappcontrol_SRCS currentappcontrol.cpp)

kde4_add_ui_files(currentappcontrol_SRCS general.ui )

kde4_add_plugin(plasma_applet_currentappcontrol ${currentappcontrol_SRCS})
target_link_libraries(plasma_applet_currentappcontrol 
                      ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${X11_XTest_LIB} ${X11_X11_LIB} ${X11_LIBS})

install(TARGETS plasma_applet_currentappcontrol
        DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES plasma-applet-currentappcontrol.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})
