include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/shells/common)

set(appletbrowserdialog_SRCS
    ../common/appletbrowser.cpp
    ../common/customdragtreeview.cpp
    ../common/kcategorizeditemsview.cpp
    ../common/kcategorizeditemsviewdelegate.cpp
    ../common/kcategorizeditemsviewmodels.cpp
    ../common/openwidgetassistant.cpp
    ../common/plasmaappletitemmodel.cpp
)

kde4_add_ui_files (
    appletbrowserdialog_SRCS
    ../common/kcategorizeditemsviewbase.ui
)

set(plasma-overlay_SRCS
    savercorona.cpp
    saverview.cpp
    main.cpp
    plasmaapp.cpp
    ${appletbrowserdialog_SRCS}
)

add_subdirectory(containment)

set(plasmaapp_dbusXML org.kde.plasma-overlay.App.xml)
#qt4_generate_dbus_interface(plasmaapp.h ${plasmaapp_dbusXML} OPTIONS -S -M)
qt4_add_dbus_adaptor(plasma-overlay_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

kde4_add_ui_files(plasma-overlay_SRCS saverconfig.ui)
kde4_add_executable(plasma-overlay ${plasma-overlay_SRCS})

target_link_libraries(plasma-overlay ${PLASMA_LIBS} kworkspace  ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS})
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-overlay ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)
set_target_properties(plasma-overlay PROPERTIES OUTPUT_NAME plasma-overlay)

install(TARGETS plasma-overlay ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-overlayrc DESTINATION ${CONFIG_INSTALL_DIR})
