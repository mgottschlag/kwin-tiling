project(qguiplatformplugin_kde)

set(kde_SOURCES  qguiplatformplugin_kde.cpp)
qt4_generate_moc(qguiplatformplugin_kde.cpp ${CMAKE_CURRENT_BINARY_DIR}/qguiplatformplugin_kde.moc)


add_library (kde SHARED ${kde_SOURCES})

target_link_libraries(kde ${KDE4_KIO_LIBS} ${QT_QTGUI_LIBRARY})

install (TARGETS kde LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/gui_platform)




