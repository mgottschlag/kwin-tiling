add_subdirectory(tests)

set(appletbrowserdialog_SRCS
    widgetsExplorer/kcategorizeditemsviewmodels.cpp
    widgetsExplorer/plasmaappletitemmodel.cpp
    widgetsExplorer/appleticon.cpp
    widgetsExplorer/appletsfiltering.cpp
    widgetsExplorer/applettooltip.cpp
    widgetsExplorer/appletslist.cpp
    widgetsExplorer/managewidgets.cpp
    widgetsExplorer/widgetexplorer.cpp
    widgetsExplorer/openwidgetassistant.cpp
)

set(plasmagenericshell_LIB_SRCS
    backgrounddialog.cpp
    mouseinputbutton.cpp
    mouseplugins.cpp
    mousepluginwidget.cpp
    wallpaperpreview.cpp
    ${appletbrowserdialog_SRCS}
)

kde4_add_ui_files(plasmagenericshell_LIB_SRCS ActivityConfiguration.ui BackgroundDialog.ui MousePlugins.ui MousePluginWidget.ui)

kde4_add_library(plasmagenericshell SHARED ${plasmagenericshell_LIB_SRCS})

target_link_libraries(plasmagenericshell ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBS} kworkspace)

set_target_properties(plasmagenericshell PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(TARGETS plasmagenericshell EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS} )
