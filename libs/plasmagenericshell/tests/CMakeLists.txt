if (KDE4_BUILD_TESTS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/../widgetsexplorer)

add_definitions(-DPLASMAGENERICSHELL_EXPORT="")

set(appletexplorer_SRCS
   ../widgetsexplorer/kcategorizeditemsviewmodels.cpp
   ../widgetsexplorer/plasmaappletitemmodel.cpp
   ../widgetsexplorer/widgetexplorer.cpp
   ../widgetsexplorer/openwidgetassistant.cpp
   main.cpp
   )
kde4_add_executable(plasmaappletexplorer ${appletexplorer_SRCS})
target_link_libraries(plasmaappletexplorer ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBRARY} ${QT_QTDECLARATIVE_LIBRARY} kephal)

set(backgrounddialog_SRCS
    background.cpp
    ../backgrounddialog.cpp
    ../mouseinputbutton.cpp
    ../mouseplugins.cpp
    ../mousepluginwidget.cpp
    ../wallpaperpreview.cpp
)

kde4_add_ui_files(backgrounddialog_SRCS ../MousePlugins.ui ../MousePlugins.ui ../BackgroundDialog.ui)
kde4_add_kcfg_files(backgrounddialog_SRCS ../../../plasma/desktop/shell/data/plasma-shell-desktop.kcfgc)

kde4_add_executable(plasmabackgrounddialog ${backgrounddialog_SRCS})
target_link_libraries(plasmabackgrounddialog ${KDE4_PLASMA_LIBS} ${KDE4_KFILE_LIBRARY} kworkspace)

endif (KDE4_BUILD_TESTS)

