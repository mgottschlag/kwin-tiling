SET(CMAKE_REQUIRED_LIBRARIES -lXi)
CHECK_FUNCTION_EXISTS(_XiGetDevicePresenceNotifyEvent XiGetDevicePresenceNotifyEvent_EXISTS)

if( X11_Xinput_FOUND AND XiGetDevicePresenceNotifyEvent_EXISTS )
    set(XiGetDevicePresence_FOUND 1)
    ADD_DEFINITIONS(-DHAVE_XINPUT_AND_DEVICE_NOTIFY=1)
else( X11_Xinput_FOUND AND XiGetDevicePresenceNotifyEvent_EXISTS )
    MESSAGE(STATUS "_XiGetDevicePresenceNotifyEvent is not found, libXi version 1.2.0 or later is required to keep layouts with keyboard hotplugging")
endif( X11_Xinput_FOUND AND XiGetDevicePresenceNotifyEvent_EXISTS )

set ( text_paint_LIB ${KDE4_PLASMA_LIBS} )



### kded daemon ###

set( kded_keyboard_SRCS
    keyboard_daemon.cpp
    layout_memory.cpp
    layout_memory_persister.cpp
    x11_helper.cpp
    xinput_helper.cpp
    xkb_helper.cpp
    keyboard_config.cpp
    keyboard_hardware.cpp
    numlockx.c
    bindings.cpp
# next are temporary for sys tray UI
    layout_tray_icon.cpp
    layouts_menu.cpp
    flags.cpp
    xkb_rules.cpp
)

kde4_add_plugin(kded_keyboard ${kded_keyboard_SRCS})

target_link_libraries(kded_keyboard
    ${KDE4_KDECORE_LIBS}  ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES}
    ${text_paint_LIB}
    ${QT_QTXML_LIBRARY} ${X11_Xkbfile_LIB}
 )

if(X11_XTest_FOUND)
  target_link_libraries(kded_keyboard ${X11_XTest_LIB})
endif(X11_XTest_FOUND)

if(XiGetDevicePresence_FOUND)
  target_link_libraries(kded_keyboard ${X11_Xinput_LIB})
endif(XiGetDevicePresence_FOUND)

install( TARGETS kded_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded )


### plasma applet ###

set( plasma_applet_keyboard_SRCS
    keyboard_applet.cpp
    flags.cpp
    keyboard_config.cpp
    xkb_rules.cpp
    x11_helper.cpp
    xkb_helper.cpp
    layouts_menu.cpp
)

kde4_add_plugin(plasma_applet_keyboard ${plasma_applet_keyboard_SRCS})

target_link_libraries(plasma_applet_keyboard
     ${KDE4_PLASMA_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY} ${X11_Xkbfile_LIB} ${X11_LIBRARIES}
     ${text_paint_LIB}
)

install( TARGETS plasma_applet_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES plasma_applet_keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


### widget lib ###

set( keyboard_layout_widget_SRCS
    layout_widget.cpp
    keyboard_config.cpp
    flags.cpp
    x11_helper.cpp
    xkb_helper.cpp
)

# MODULE?
kde4_add_plugin(keyboard_layout_widget ${keyboard_layout_widget_SRCS})

target_link_libraries(keyboard_layout_widget
     ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES}
     ${text_paint_LIB}
     ${QT_QTXML_LIBRARY} ${X11_Xkbfile_LIB}
)

install( TARGETS keyboard_layout_widget DESTINATION ${PLUGIN_INSTALL_DIR} )


### kcm keyboard ###

set(kcm_keyboard_PART_SRCS 
  kcm_keyboard.cpp
  kcm_keyboard_widget.cpp
  kcm_view_models.cpp
  kcm_add_layout_dialog.cpp
  keyboard_config.cpp
  x11_helper.cpp
  xkb_helper.cpp
  xkb_rules.cpp
  flags.cpp
  iso_codes.cpp
  kcmmisc.cpp
  bindings.cpp
)


kde4_add_ui_files(kcm_keyboard_PART_SRCS kcm_keyboard.ui kcm_add_layout_dialog.ui kcmmiscwidget.ui)

kde4_add_plugin(kcm_keyboard ${kcm_keyboard_PART_SRCS})

target_link_libraries(kcm_keyboard
    ${KDE4_KIO_LIBS} ${X11_Xkbfile_LIB} ${X11_LIBRARIES}
    ${text_paint_LIB}
)

install(TARGETS kcm_keyboard DESTINATION ${PLUGIN_INSTALL_DIR} )

install( FILES kcm_keyboard.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

# Images

add_subdirectory( pics )


# Unit tests

add_subdirectory( tests )
