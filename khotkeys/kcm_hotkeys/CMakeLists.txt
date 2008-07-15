# Qt Designer Files
include_directories(ui)

set(
    kcm_hotkeys_PART_SRCS
    kcm_hotkeys.cpp
    kcm_gestures.cpp
    kcm_module_factory.cpp
    global_settings_widget.cpp
    hotkeys_model.cpp
    hotkeys_proxy_model.cpp
    hotkeys_tree_view.cpp

    # TRIGGER WIDGETS
    trigger_widget_base.cpp
    shortcut_trigger_widget.cpp

    # ACTION WIDGETS
    action_widget_base.cpp
    command_url_action_widget.cpp
    dbus_action_widget.cpp
    menuentry_action_widget.cpp

    # ACTION_DATA WIDGETS
    hotkeys_widget_iface.cpp
    hotkeys_widget_base.cpp
    action_group_widget.cpp
    simple_action_data_widget.cpp
    )

kde4_add_ui_files(
    kcm_hotkeys_PART_SRCS
    ui/global_settings_widget.ui
    ui/command_url_action_widget.ui
    ui/dbus_action_widget.ui
    ui/hotkeys_widget_base.ui
    ui/simple_action_data_widget.ui
    ui/shortcut_trigger_widget.ui
    ui/menuentry_action_widget.ui
    )

kde4_add_plugin(
    kcm_hotkeys
    ${kcm_hotkeys_PART_SRCS}
    )

target_link_libraries(
    kcm_hotkeys
    khotkeysprivate
    ${KDE4_KIO_LIBS}
    ${QT_QTGUI_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    )

install(
    TARGETS kcm_hotkeys
    DESTINATION ${PLUGIN_INSTALL_DIR}
    )

install(
    FILES khotkeys.desktop
          gestures.desktop
          settings-input-actions.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )
