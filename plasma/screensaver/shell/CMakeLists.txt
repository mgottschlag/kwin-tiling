include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell)

set(plasma-overlay_SRCS
    backgrounddialog.cpp
    main.cpp
    plasmaapp.cpp
    savercorona.cpp
    saverview.cpp
)

kde4_add_ui_files(plasma-overlay_SRCS BackgroundDialog.ui)

set(plasmaapp_dbusXML org.kde.plasma-overlay.App.xml)
#qt4_generate_dbus_interface(plasmaapp.h ${plasmaapp_dbusXML} OPTIONS -S -M)
qt4_add_dbus_adaptor(plasma-overlay_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

kde4_add_executable(plasma-overlay ${plasma-overlay_SRCS})

target_link_libraries(plasma-overlay ${KDE4_PLASMA_LIBS} kworkspace  ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS}
                                     ${X11_X11_LIB} plasmagenericshell ${QT_QTDECLARATIVE_LIBRARY} kdeclarative)
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-overlay ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)
set_target_properties(plasma-overlay PROPERTIES OUTPUT_NAME plasma-overlay)

install(TARGETS plasma-overlay ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-overlayrc DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES qml/lockscreen.qml DESTINATION ${DATA_INSTALL_DIR}/plasma/screenlocker/)
