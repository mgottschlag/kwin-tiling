kde4_header()

project(khotkeys)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/gestures.cpp )


add_subdirectory( shared ) 
add_subdirectory( kcontrol ) 
add_subdirectory( app ) 
add_subdirectory( data ) 
add_subdirectory( update ) 
include_directories(  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} )


########### install files ###############

install_files( /share/autostart FILES khotkeys.desktop )

kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = shared kcontrol app data update
#
#khotkeys_autostart_DATA = khotkeys.desktop
#khotkeys_autostartdir = $(datadir)/autostart
#
#EXTRA_DIST = $(khotkeys_autostart_DATA) COPYING AUTHORS
#
#messages: rc.cpp
#	$(EXTRACTRC) kcontrol/ui/*.ui data/*.khotkeys >> rc.cpp
#	$(XGETTEXT) rc.cpp app/*.cpp shared/*.cpp shared/*.h kcontrol/*.cpp -o $(podir)/khotkeys.pot
