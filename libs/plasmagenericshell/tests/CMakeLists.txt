if (KDE4_BUILD_TESTS)

include_directories(../ ../widgetsExplorer ../../../plasma/shells/desktop ${CMAKE_CURRENT_BINARY_DIR}/../../../plasma/shells/desktop)

set(appletexplorer_SRCS
   ../../../plasma/shells/common/widgetsExplorer/kcategorizeditemsviewmodels.cpp
   ../../../plasma/shells/common/widgetsExplorer/plasmaappletitemmodel.cpp
   ../../../plasma/shells/common/widgetsExplorer/appleticon.cpp
   ../../../plasma/shells/common/widgetsExplorer/appletsfiltering.cpp
   ../../../plasma/shells/common/widgetsExplorer/applettooltip.cpp
   ../../../plasma/shells/common/widgetsExplorer/appletslist.cpp
   ../../../plasma/shells/common/widgetsExplorer/managewidgets.cpp
   ../../../plasma/shells/common/widgetsExplorer/widgetexplorer.cpp
   ../../../plasma/shells/common/widgetsExplorer/openwidgetassistant.cpp
   main.cpp
   )

kde4_add_executable(plasmaappletexplorer ${appletexplorer_SRCS})
target_link_libraries(plasmaappletexplorer ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBRARY})

set(backgrounddialog_SRCS
    background.cpp
    ../backgrounddialog.cpp
    ../mouseinputbutton.cpp
    ../mouseplugins.cpp
    ../mousepluginwidget.cpp
    ../wallpaperpreview.cpp
)

kde4_add_ui_files(backgrounddialog_SRCS ../MousePlugins.ui ../MousePluginWidget.ui ../BackgroundDialog.ui ../ActivityConfiguration.ui)
kde4_add_kcfg_files(backgrounddialog_SRCS ../../../plasma/shells/desktop/kcfg/plasma-shell-desktop.kcfgc)

kde4_add_executable(plasmabackgrounddialog ${backgrounddialog_SRCS})
target_link_libraries(plasmabackgrounddialog ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KFILE_LIBRARY} kworkspace)

endif (KDE4_BUILD_TESTS)

