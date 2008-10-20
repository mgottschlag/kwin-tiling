project(webbrowser)

set(webbrowser_SRCS
    historycombobox.cpp
    bookmarksdelegate.cpp
    bookmarkitem.cpp
    webbrowser.cpp)

kde4_add_ui_files(webbrowser_SRCS webbrowserconfig.ui )
kde4_add_plugin(plasma_applet_webbrowser ${webbrowser_SRCS})
target_link_libraries(plasma_applet_webbrowser plasma ${KDE4_KDEUI_LIBS} ${QT_QTWEBKIT_LIBRARY} ${KDE4_KIO_LIBS} ${KDE4_KPARTS_LIBS})

install(TARGETS plasma_applet_webbrowser DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-applet-webbrowser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
