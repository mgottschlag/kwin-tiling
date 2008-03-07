
# Check for covariant return type
check_cxx_source_compiles("
  class A { public: virtual A* me(); };
  class X { public: int x; virtual void ff() {}; };
  class B : public X, public A { public: virtual B* me(); };
  int foo( A* a )
    {
    A* aa = a->me();
    return a == aa;
    }
  int main()
    {
    B* b = new B;
    return foo( b ) == 0;
    }
  A* A::me() { return this; }
  B* B::me() { return this; }
" HAVE_COVARIANT_RETURN )

configure_file(config-khotkeys.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-khotkeys.h )

# KHotKey convenience library. Contains
# - Actions
# - Trigger
# - Conditions
# - Some gui stuff
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(libkhotkeys_SRCS

    # ACTIONS
    actions/action.cpp
    actions/action_data.cpp
    actions/action_list.cpp
    actions/activate_window_action.cpp
    actions/command_url_action.cpp
    actions/dbus_action.cpp
    actions/keyboard_input_action.cpp
    actions/menuentry_action.cpp

    # CONDITIONS
    conditions/active_window_condition.cpp
    conditions/and_condition.cpp
    conditions/condition.cpp
    conditions/conditions.cpp
    conditions/conditions_list_base.cpp
    conditions/conditions_list.cpp
    conditions/existing_window_condition.cpp
    conditions/not_condition.cpp
    conditions/or_condition.cpp

    # TRIGGERS
    triggers/gestures.cpp
    triggers/gesture_trigger.cpp
    triggers/shortcut_trigger.cpp
    triggers/trigger.cpp
    triggers/trigger_list.cpp
    triggers/triggers.cpp
    # triggers/voice_trigger.cpp
    triggers/window_trigger.cpp

    # Rest
    input.cpp
    khlistbox.cpp
    khlistview.cpp
    khotkeysglobal.cpp
    settings.cpp
    # search for FIXME: SOUND
    # sound.cpp
    # soundrecorder.cpp
    # voices.cpp
    # voicesignature.cpp
    windows.cpp
)

# We have a khotkeys executable target, so we have to do some cmake magic here
# to get a libkhotkeys
kde4_add_library(
    libkhotkeys
    SHARED
    ${libkhotkeys_SRCS} )
set_target_properties(
    libkhotkeys
    PROPERTIES OUTPUT_NAME "khotkeys")
target_link_libraries(
    libkhotkeys
    kworkspace
    ${KDE4_KDE3SUPPORT_LIBS} )

########### install files ###############

install( TARGETS libkhotkeys DESTINATION ${LIB_INSTALL_DIR} )

