include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ../../../shells/common ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

set(appletbrowserdialog_SRCS
    ../../../shells/common/widgetsExplorer/kcategorizeditemsviewmodels.cpp
    ../../../shells/common/widgetsExplorer/plasmaappletitemmodel.cpp
    ../../../shells/common/widgetsExplorer/appleticon.cpp
    ../../../shells/common/widgetsExplorer/appletsfiltering.cpp
    ../../../shells/common/widgetsExplorer/applettooltip.cpp
    ../../../shells/common/widgetsExplorer/appletslist.cpp
    ../../../shells/common/widgetsExplorer/managewidgets.cpp
    ../../../shells/common/widgetsExplorer/widgetexplorer.cpp
    ../../../shells/common/widgetsExplorer/openwidgetassistant.cpp
)


set(plasma-netbook_SRCS
    netcorona.cpp
    netview.cpp
    main.cpp
    plasmaapp.cpp
    ../../../shells/common/backgrounddialog.cpp
    ${appletbrowserdialog_SRCS}
)

kde4_add_ui_files(plasma-netbook_SRCS ../../../shells/common/BackgroundDialog.ui ../../../shells/common/ActivityConfiguration.ui )

#set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
set(ksmserver_xml ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma-netbook_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-netbook ${plasma-netbook_SRCS})

target_link_libraries(kdeinit_plasma-netbook ${KDE4_PLASMA_LIBS} kworkspace ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KDE4_KFILE_LIBS} kephal)
if(X11_Xrender_FOUND)
   target_link_libraries(kdeinit_plasma-netbook ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

install(TARGETS kdeinit_plasma-netbook DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-netbook ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-default-layoutrc DESTINATION ${DATA_INSTALL_DIR}/plasma-netbook/)
#install(FILES plasma-netbook.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

