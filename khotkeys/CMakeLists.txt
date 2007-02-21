project(khotkeys)


check_cxx_source_compiles("
  class A { public: virtual A* me(); };
  class X { public: int x; virtual void ff() {}; };
  class B : public X, public A { public: virtual B* me(); };
  int foo( A* a )
    {·
    A* aa = a->me();
    return a == aa;
    }·
  int main()
    {·
    B* b = new B;
    return foo( b ) == 0;
    }·
  A* A::me() { return this; }
  B* B::me() { return this; }
" HAVE_COVARIANT_RETURN )
configure_file(config-khotkeys.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-khotkeys.h )
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(libshared_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/action_data.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/khotkeysglobal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/triggers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/windows.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/settings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/actions.cpp
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


add_subdirectory( shared )
add_subdirectory( kcontrol )
add_subdirectory( app )
add_subdirectory( data )
add_subdirectory( update )



########### install files ###############

install( FILES khotkeys.desktop DESTINATION ${AUTOSTART_INSTALL_DIR} )

