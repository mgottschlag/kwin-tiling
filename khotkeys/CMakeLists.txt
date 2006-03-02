kde4_header()

add_subdirectory( shared ) 
add_subdirectory( kcontrol ) 
add_subdirectory( app ) 
add_subdirectory( data ) 
add_subdirectory( update ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )

set(libshared_SRCS
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/action_data.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/khotkeysglobal.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/triggers.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/windows.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/settings.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/actions.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/input.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/conditions.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/khlistview.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/khlistbox.cpp
    ${CMAKE_SOURCE_DIR}/khotkeys/shared/gestures.cpp )


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
