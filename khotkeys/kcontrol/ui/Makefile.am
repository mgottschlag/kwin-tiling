noinst_LTLIBRARIES = libui.la
libui_la_SOURCES = dummy.cpp windowdef_simple_widget_ui.ui windowdef_list_widget_ui.ui \
    command_url_widget_ui.ui menuentry_widget_ui.ui triggers_tab_ui.ui general_tab_ui.ui \
    action_group_tab_ui.ui dcop_widget_ui.ui main_buttons_widget_ui.ui \
    actions_listview_widget_ui.ui info_tab_ui.ui window_trigger_widget_ui.ui \
    action_list_widget_ui.ui keyboard_input_widget_ui.ui condition_list_widget_ui.ui \
    gesture_triggers_tab_ui.ui gestures_settings_tab_ui.ui general_settings_tab_ui.ui \
    voice_input_widget_ui.ui voice_settings_tab_ui.ui

libui_la_LIBADD  = ../../shared/libkhotkeys_shared.la $(LIB_KDEUI)
libui_la_LDFLAGS = $(all_libraries)

INCLUDES= -I$(srcdir)/.. -I$(top_srcdir)/khotkeys/shared $(all_includes)

dummy.cpp:
	echo > dummy.cpp

DISTCLEANFILES = dummy.cpp

METASOURCES = AUTO

