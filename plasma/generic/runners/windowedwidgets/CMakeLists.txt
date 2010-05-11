project(windowedwidgetsrunner)

include_directories( ${QT_INCLUDES} ${KDE4_INCLUDES} )

set(krunner_windowedwidgets_SRCS
    windowedwidgetsrunner.cpp
)

kde4_add_plugin(krunner_windowedwidgets ${krunner_windowedwidgets_SRCS})
target_link_libraries(krunner_windowedwidgets ${KDE4_PLASMA_LIBS})

install(TARGETS krunner_windowedwidgets DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-runner-windowedwidgets.desktop DESTINATION ${SERVICES_INSTALL_DIR})
