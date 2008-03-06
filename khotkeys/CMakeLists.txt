

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
include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDEBASE_WORKSPACE_SOURCE_DIR}/khotkeys/libkhotkeys 
    )

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libkhotkeys_SRCS

    # ACTIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/action.cpp       
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/action_list.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/activate_window_action.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/command_url_action.cpp      
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/dbus_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/keyboard_input_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/menuentry_action.cpp

    # CONDITIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/active_window_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/and_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/conditions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/conditions_list_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/conditions_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/existing_window_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/not_condition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/or_condition.cpp

    # TRIGGERS
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/gesture_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/shortcut_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/trigger_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/triggers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/voice_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/window_trigger.cpp

    # Rest
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/action_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/khotkeysglobal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/khlistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/khlistbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/gestures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/soundrecorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/voicesignature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libkhotkeys/voices.cpp
)

# Convenience library for kdeinit, kpart etc.
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

add_subdirectory( kcontrol )
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )



########### install files ###############

install( TARGETS libkhotkeys DESTINATION ${LIB_INSTALL_DIR} )
install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )

