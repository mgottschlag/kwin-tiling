
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

install( FILES khotkeys.desktop DESTINATION ${DATA_INSTALL_DIR}/autostart )




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
