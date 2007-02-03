
include_directories( ${CMAKE_SOURCE_DIR}/workspace/khotkeys/shared ui/  )


########### next target ###############

set(kcm_khotkeys_PART_SRCS ${libshared_SRCS}
   menuedit.cpp 
   window_trigger_widget.cpp 
   tab_widget.cpp 
   main_buttons_widget.cpp 
   actions_listview.cpp 
   actions_listview_widget.cpp 
   menuentry_widget.cpp 
   general_tab.cpp 
   action_group_tab.cpp 
   kcmkhotkeys.cpp 
   command_url_widget.cpp 
   windowdef_list_widget.cpp 
   windowdef_simple_widget.cpp 
   triggers_tab.cpp 
   dcop_widget.cpp 
   info_tab.cpp 
   action_list_widget.cpp 
   keyboard_input_widget.cpp 
   condition_list_widget.cpp 
   gesturedrawer.cpp 
   gesturerecorder.cpp 
   gesturerecordpage.cpp 
   gesture_triggers_tab.cpp 
   windowselector.cpp 
   gestures_settings_tab.cpp 
   general_settings_tab.cpp
    voice_settings_tab.cpp
    voicerecorder.cpp
    voicerecordpage.cpp)


kde4_add_ui3_files(kcm_khotkeys_PART_SRCS
	ui/windowdef_simple_widget_ui.ui 
	ui/windowdef_list_widget_ui.ui 
	ui/command_url_widget_ui.ui 
	ui/menuentry_widget_ui.ui 
	ui/triggers_tab_ui.ui 
	ui/general_tab_ui.ui 
	ui/action_group_tab_ui.ui 
	ui/dcop_widget_ui.ui 
	ui/main_buttons_widget_ui.ui 
	ui/actions_listview_widget_ui.ui 
	ui/info_tab_ui.ui 
	ui/window_trigger_widget_ui.ui 
	ui/action_list_widget_ui.ui 
	ui/keyboard_input_widget_ui.ui 
	ui/condition_list_widget_ui.ui 
	ui/gesture_triggers_tab_ui.ui 
	ui/gestures_settings_tab_ui.ui 
	ui/general_settings_tab_ui.ui
    ui/voice_settings_tab_ui.ui
    ui/voice_input_widget_ui.ui)

# run moc on actions_listview.h and add the resulting stuff to the kcm_khotkeys_PART_SRCS list
qt4_wrap_cpp(kcm_khotkeys_PART_SRCS actions_listview.h)

kde4_automoc(${kcm_khotkeys_PART_SRCS})

kde4_add_plugin(kcm_khotkeys ${kcm_khotkeys_PART_SRCS})


target_link_libraries(kcm_khotkeys  ${KDE4_KDE3SUPPORT_LIBS} )
if(X11_XTest_FOUND)
  target_link_libraries(kcm_khotkeys  ${X11_XTest_LIB} )
endif(X11_XTest_FOUND)

install(TARGETS kcm_khotkeys  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES khotkeys.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )

kde4_install_icons( ${ICON_INSTALL_DIR}  )


