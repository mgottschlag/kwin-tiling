if (KDE4_BUILD_TESTS)

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

include_directories(../)
kde4_add_executable(plasmaappletbrowser ${appletbrowser_SRCS})
target_link_libraries(plasmaappletbrowser plasma ${KDE4_KDEUI_LIBS})

endif (KDE4_BUILD_TESTS)

