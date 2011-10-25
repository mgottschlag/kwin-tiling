set(
    kcm_hotkeys_PART_SRCS
    kcm_hotkeys.cpp
    kcm_module_factory.cpp
    global_settings_widget.cpp
    hotkeys_export_widget.cpp
    hotkeys_model.cpp
    hotkeys_proxy_model.cpp
    hotkeys_tree_view.cpp
    hotkeys_context_menu.cpp

    # HELPER WIDGETS
    helper_widgets/window_selector.cpp
    helper_widgets/window_definition_widget.cpp
    helper_widgets/window_definition_list_widget.cpp
    helper_widgets/gesture_drawer.cpp
    helper_widgets/gesture_recorder.cpp
    helper_widgets/gesture_widget.cpp
    helper_widgets/edit_gesture_dialog.cpp

    # TRIGGER WIDGETS
    triggers/trigger_widget_base.cpp
    triggers/shortcut_trigger_widget.cpp
    triggers/window_trigger_widget.cpp
    triggers/gesture_trigger_widget.cpp

    # ACTION WIDGETS
    actions/action_widget_base.cpp
    actions/command_url_action_widget.cpp
    actions/dbus_action_widget.cpp
    actions/menuentry_action_widget.cpp
    actions/keyboard_input_action_widget.cpp

    # CONDITION WiDGETS
    conditions/conditions_widget.cpp
    conditions/condition_type_menu.cpp

    # ACTION_DATA WIDGETS
    hotkeys_widget_iface.cpp
    hotkeys_widget_base.cpp
    action_group_widget.cpp
    simple_action_data_widget.cpp
    )

kde4_add_ui_files(
    kcm_hotkeys_PART_SRCS

    global_settings_widget.ui
    hotkeys_export_widget.ui
    kcm_hotkeys.ui

    # HELPER WIDGETS
    helper_widgets/window_definition_list_widget.ui
    helper_widgets/window_definition_widget.ui
    helper_widgets/gesture_widget.ui

    # TRIGGER WIDGETS
    triggers/window_trigger_widget.ui
    triggers/shortcut_trigger_widget.ui
    triggers/gesture_trigger_widget.ui

    # ACTION WIDGETS
    actions/command_url_action_widget.ui
    actions/dbus_action_widget.ui
    actions/menuentry_action_widget.ui
    actions/keyboard_input_action_widget.ui

    # CONDITION WiDGETS
    conditions/conditions_widget.ui

    # ACTION_DATA WIDGETS
    hotkeys_widget_base.ui
    )

qt4_add_dbus_interface(
    kcm_hotkeys_PART_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/../app/org.kde.khotkeys.xml
    khotkeys_interface)


kde4_add_plugin(
    kcm_hotkeys
    ${kcm_hotkeys_PART_SRCS}
    )

add_dependencies(kcm_hotkeys kded_khotkeys)

target_link_libraries(
    kcm_hotkeys
    khotkeysprivate
    ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${X11_X11_LIB}
    )

install(
    TARGETS kcm_hotkeys
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )

install(
    FILES khotkeys.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )
