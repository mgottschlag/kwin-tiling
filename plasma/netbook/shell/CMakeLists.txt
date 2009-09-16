include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/plasmagenericshell ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kephal ${KDEBASE_WORKSPACE_SOURCE_DIR}/libs/kworkspace)


set(plasma-netbook_SRCS
    netcorona.cpp
    netview.cpp
    main.cpp
    plasmaapp.cpp
)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma-netbook_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-netbook ${plasma-netbook_SRCS})

target_link_libraries(kdeinit_plasma-netbook ${KDE4_PLASMA_LIBS} kworkspace ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES} ${KDE4_KFILE_LIBS} kephal plasmagenericshell)
if(X11_Xrender_FOUND)
   target_link_libraries(kdeinit_plasma-netbook ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)

install(TARGETS kdeinit_plasma-netbook DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-netbook ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-default-layoutrc DESTINATION ${DATA_INSTALL_DIR}/plasma-netbook/)
#install(FILES plasma-netbook.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

