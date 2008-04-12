set(qscript_engine_SRCS
    appletscriptqscript.cpp
    qtgui/font.cpp
    qtgui/graphicsitem.cpp
    qtgui/painter.cpp
    qtgui/point.cpp
    qtgui/rect.cpp
    qtgui/size.cpp
    qtgui/timer.cpp
)

kde4_add_plugin(plasma_scriptengine_qscript ${qscript_engine_SRCS})
target_link_libraries(plasma_scriptengine_qscript ${KDE4_KDECORE_LIBS} plasma ${QT_QTSCRIPT_LIBRARY} ${QT_QTUITOOLS_LIBRARY})

install(TARGETS plasma_scriptengine_qscript DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-scriptengine-qscript.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

