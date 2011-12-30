add_subdirectory(tests)

set(appletbrowserdialog_SRCS
    widgetsexplorer/kcategorizeditemsviewmodels.cpp
    widgetsexplorer/plasmaappletitemmodel.cpp
    widgetsexplorer/widgetexplorer.cpp
    widgetsexplorer/openwidgetassistant.cpp
)

set(scripting_SRC
    scripting/appinterface.cpp
    scripting/applet.cpp
    scripting/containment.cpp
    scripting/i18n.cpp
    scripting/layouttemplatepackagestructure.cpp
    scripting/rect.cpp
    scripting/scriptengine.cpp
    scripting/widget.cpp
)

set(plasmagenericshell_LIB_SRCS
    backgrounddialog.cpp
    mouseinputbutton.cpp
    mouseplugins.cpp
    mousepluginwidget.cpp
    panelshadows.cpp
    toolbutton.cpp
    wallpaperpreview.cpp
    ${scripting_SRC}
    ${appletbrowserdialog_SRCS}
)

kde4_add_ui_files(plasmagenericshell_LIB_SRCS BackgroundDialog.ui MousePlugins.ui)

kde4_add_library(plasmagenericshell SHARED ${plasmagenericshell_LIB_SRCS})

target_link_libraries(plasmagenericshell ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} ${QT_QTSCRIPT_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} kworkspace kephal)
if(UNIX AND Q_WS_X11)
    target_link_libraries(plasmagenericshell ${X11_LIBRARIES})
endif(UNIX AND Q_WS_X11)

set_target_properties(plasmagenericshell PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(DIRECTORY widgetsexplorer/package/ DESTINATION ${DATA_INSTALL_DIR}/plasma/packages/org.kde.desktop.widgetexplorer)

install(TARGETS plasmagenericshell EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
INSTALL(FILES plasma-layout-template.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

