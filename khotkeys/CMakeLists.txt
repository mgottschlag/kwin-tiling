

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
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

set(libkhotkeys_SRCS

    # ACTIONS
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/action.cpp       
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/action_list.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/activate_window_action.cpp  
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/command_url_action.cpp      
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/dbus_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/keyboard_input_action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/menuentry_action.cpp

    # TRIGGERS
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/gesture_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/shortcut_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/trigger_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/triggers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/voice_trigger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/window_trigger.cpp

    # Rest
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/action_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khotkeysglobal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/conditions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khlistview.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khlistbox.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/gestures.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/soundrecorder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/voicesignature.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sound.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/voices.cpp
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

