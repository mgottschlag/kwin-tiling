if (KDE4_BUILD_TESTS)

include_directories(../ ../../desktop ${CMAKE_CURRENT_BINARY_DIR}/../../desktop)

set(appletbrowser_SRCS
    appletbrowser.cpp
    ../appletbrowser.cpp
    ../customdragtreeview.cpp
    ../kcategorizeditemsview.cpp
    ../kcategorizeditemsviewdelegate.cpp
    ../kcategorizeditemsviewmodels.cpp
    ../openwidgetassistant.cpp
    ../plasmaappletitemmodel.cpp
    )

kde4_add_ui_files (
    appletbrowser_SRCS
    ../kcategorizeditemsviewbase.ui
)

kde4_add_executable(plasmaappletbrowser ${appletbrowser_SRCS})
target_link_libraries(plasmaappletbrowser ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${KDE4_KFILE_LIBRARY})

set(backgrounddialog_SRCS
    background.cpp
    ../../desktop/backgrounddialog.cpp
    ../../desktop/mouseinputbutton.cpp
    ../../desktop/mouseplugins.cpp
    ../../desktop/mousepluginwidget.cpp
    ../wallpaperpreview.cpp
)

kde4_add_ui_files(backgrounddialog_SRCS ../../desktop/MousePlugins.ui)
kde4_add_ui_files(backgrounddialog_SRCS ../../desktop/BackgroundDialog.ui)
kde4_add_kcfg_files(backgrounddialog_SRCS ../../desktop/kcfg/plasma-shell-desktop.kcfgc)

kde4_add_executable(plasmabackgrounddialog ${backgrounddialog_SRCS})
target_link_libraries(plasmabackgrounddialog ${KDE4_PLASMA_LIBS} ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KFILE_LIBRARY} kworkspace)

endif (KDE4_BUILD_TESTS)

