include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell ${KDEBASE_WORKSPACE_SOURCE_DIR}/plasma/netbook/shell/scripting/ ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)

add_subdirectory(nettoolbox)

set(plasma-netbook_SRCS
    scripting/netbookscriptengine.cpp
    scripting/panel.cpp
    scripting/newspaper.cpp
    netcorona.cpp
    netview.cpp
    main.cpp
    plasmaapp.cpp
    netpanelcontroller.cpp
    netdialogmanager.cpp
)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma-netbook_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-netbook ${plasma-netbook_SRCS})

target_link_libraries(kdeinit_plasma-netbook ${KDE4_PLASMA_LIBS} kworkspace ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KDE4_KFILE_LIBS} kephal plasmagenericshell ${QT_QTOPENGL_LIBRARY} ${QT_QTSCRIPT_LIBRARY})
if(X11_Xrender_FOUND)
   target_link_libraries(kdeinit_plasma-netbook ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

install(TARGETS kdeinit_plasma-netbook DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-netbook ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES data/layouts/00-defaultLayout.js DESTINATION ${DATA_INSTALL_DIR}/plasma-netbook/init)

install(FILES data/layouts/org.kde.plasma-netbook.defaultSal/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultSal/)
install(FILES data/layouts/org.kde.plasma-netbook.defaultSal/contents/layout.js DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultSal/contents/)
install(FILES data/layouts/plasma-layout-org.kde.plasma-netbook.defaultSal.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/layouts/org.kde.plasma-netbook.defaultPanel/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultPanel/)
install(FILES data/layouts/org.kde.plasma-netbook.defaultPanel/contents/layout.js DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultPanel/contents/)
install(FILES data/layouts/plasma-layout-org.kde.plasma-netbook.defaultPanel.desktop DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES data/layouts/org.kde.plasma-netbook.defaultPage/metadata.desktop DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultPage/)
install(FILES data/layouts/org.kde.plasma-netbook.defaultPage/contents/layout.js DESTINATION ${DATA_INSTALL_DIR}/plasma/layout-templates/org.kde.plasma-netbook.defaultPage/contents/)
install(FILES data/layouts/plasma-layout-org.kde.plasma-netbook.defaultPage.desktop DESTINATION ${SERVICES_INSTALL_DIR})
