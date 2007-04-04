

add_definitions (-DQT3_SUPPORT)

########### next target ###############

set(kcm_joystick_PART_SRCS 
   joystick.cpp 
   joywidget.cpp 
   poswidget.cpp 
   joydevice.cpp 
   caldialog.cpp )

kde4_automoc(${kcm_joystick_PART_SRCS})

kde4_add_plugin(kcm_joystick ${kcm_joystick_PART_SRCS})


target_link_libraries(kcm_joystick  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kcm_joystick  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES joystick.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

#
