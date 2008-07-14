include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

set(plasma-overlay_SRCS
    savercorona.cpp
    saverview.cpp
    main.cpp
    plasmaapp.cpp
)

add_subdirectory(containment)

#set(plasmaapp_dbusXML org.kde.plasma.App.xml)
#qt4_add_dbus_adaptor(plasma_SRCS ${plasmaapp_dbusXML} plasmaapp.h PlasmaApp)

#set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
#qt4_add_dbus_interface(plasma_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_executable(plasma-overlay ${plasma-overlay_SRCS})

target_link_libraries(plasma-overlay plasma kworkspace  ${KDE4_KIO_LIBS})
if(X11_Xrender_FOUND)
  target_link_libraries(plasma-overlay ${X11_Xrender_LIB})
endif(X11_Xrender_FOUND)
set_target_properties(plasma-overlay PROPERTIES OUTPUT_NAME plasma-overlay)

#install(TARGETS kdeinit_plasma-qgv DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-overlay ${INSTALL_TARGETS_DEFAULT_ARGS})
#install( FILES plasma.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )
