SUBDIRS = ui

kde_module_LTLIBRARIES = kcm_khotkeys.la

kcm_khotkeys_la_SOURCES = \
    menuedit.cpp window_trigger_widget.cpp tab_widget.cpp main_buttons_widget.cpp \
    actions_listview.cpp actions_listview_widget.cpp menuentry_widget.cpp general_tab.cpp \
    action_group_tab.cpp kcmkhotkeys.cpp command_url_widget.cpp windowdef_list_widget.cpp \
    windowdef_simple_widget.cpp triggers_tab.cpp dcop_widget.cpp info_tab.cpp \
    action_list_widget.cpp keyboard_input_widget.cpp condition_list_widget.cpp \
    gesturedrawer.cpp gesturerecorder.cpp gesturerecordpage.cpp gesture_triggers_tab.cpp \
    windowselector.cpp gestures_settings_tab.cpp general_settings_tab.cpp
    
kcm_khotkeys_la_LIBADD  = ui/libui.la $(LIB_KDEUI)
kcm_khotkeys_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined

noinst_HEADERS = menuedit.h window_trigger_widget.h \
    tab_widget.h main_buttons_widget.h actions_listview.h actions_listview_widget.h menuentry_widget.h \
    general_tab.h action_group_tab.h kcmkhotkeys.h windowdef_list_widget.h \
    command_url_widget.h windowdef_simple_widget.h triggers_tab.h dcop_widget.h info_tab.h \
    action_list_widget.h keyboard_input_widget.h condition_list_widget.h \
    activate_window_widget.h \
    gesturedrawer.h gesturerecorder.h gesturerecordpage.h gesture_triggers_tab.h \
    windowselector.h gestures_settings_tab.h general_settings_tab.h


INCLUDES= -I$(top_srcdir)/khotkeys/shared -I$(srcdir)/ui -I./ui $(all_includes)

METASOURCES = AUTO

xdg_apps_DATA = khotkeys.desktop

KDE_ICON = khotkeys
