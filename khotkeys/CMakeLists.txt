

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

configure_file( config-khotkeys.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-khotkeys.h )

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} 
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/actions 
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/conditions
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/triggers
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/shared 
    )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libhotkeys_SRCS
    # ACTIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/activate_window_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/command_url_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/dbus_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/keyboard_input_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/menuentry_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/actions/action_list.cpp

    # CONDITIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/active_window_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/and_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/conditions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/conditions_list_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/conditions_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/existing_window_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/not_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/conditions/or_condition.cpp

    # TRIGGERS
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/gesture_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/shortcut_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/triggers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/trigger_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/voice_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/triggers/window_trigger.cpp

    # SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/action_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/gestures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khlistbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khlistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khotkeysglobal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/soundrecorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/voices.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/voicesignature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/windows.cpp
)

# A internal library
kde4_add_library( 
    hotkeys 
    SHARED 
    ${libhotkeys_SRCS} )
target_link_libraries( 
    hotkeys 
    kworkspace 
    ${KDE4_KDE3SUPPORT_LIBS} )

# Build the subdirectories
add_subdirectory( kcontrol )
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )

########### install files ###############

install( TARGETS hotkeys DESTINATION ${LIB_INSTALL_DIR} )
install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )

