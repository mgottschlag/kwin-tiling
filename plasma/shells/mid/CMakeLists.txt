include_directories(${KDEBASE_WORKSPACE_SOURCE_DIR}/libs)

set(plasma-mid_SRCS
    midcorona.cpp
    midview.cpp
    main.cpp
    plasmaapp.cpp
)

set(ksmserver_xml ${KDEBASE_WORKSPACE_SOURCE_DIR}/ksmserver/org.kde.KSMServerInterface.xml)
qt4_add_dbus_interface(plasma-mid_SRCS ${ksmserver_xml} ksmserver_interface)

kde4_add_kdeinit_executable(plasma-mid ${plasma-mid_SRCS})

if(NOT WIN32)
    target_link_libraries(kdeinit_plasma-mid plasma kworkspace ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES})
    if(X11_Xrender_FOUND)
      target_link_libraries(kdeinit_plasma-mid ${X11_Xrender_LIB})
    endif(X11_Xrender_FOUND)
else(NOT WIN32)
    target_link_libraries(kdeinit_plasma-mid plasma ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KIO_LIBS} ${X11_LIBRARIES})
endif(NOT WIN32)

install(TARGETS kdeinit_plasma-mid DESTINATION ${LIB_INSTALL_DIR})
install(TARGETS plasma-mid ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES plasma-mid.desktop DESTINATION ${AUTOSTART_INSTALL_DIR})

