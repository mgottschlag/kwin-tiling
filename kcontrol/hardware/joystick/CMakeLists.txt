


########### next target ###############

set(kcm_joystick_PART_SRCS 
   joystick.cpp 
   joywidget.cpp 
   poswidget.cpp 
   joydevice.cpp 
   caldialog.cpp )

kde4_automoc(${kcm_joystick_PART_SRCS})

kde4_add_plugin(kcm_joystick ${kcm_joystick_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_joystick )

target_link_libraries(kcm_joystick  ${KDE4_KDEUI_LIBS} ${QT_QTGUI_LIBRARY})

install(TARGETS kcm_joystick  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES joystick.desktop  DESTINATION  ${XDG_APPS_DIR} )




#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes)
#METASOURCES = AUTO
#
## Install this plugin in the KDE modules directory
#kde_module_LTLIBRARIES = kcm_joystick.la
#
#kcm_joystick_la_SOURCES = joystick.cpp joywidget.cpp poswidget.cpp joydevice.cpp caldialog.cpp
#kcm_joystick_la_LIBADD = $(LIB_KDEUI)
#kcm_joystick_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
#
#xdg_apps_DATA = joystick.desktop
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/joystick.pot
#
#noinst_HEADERS =  joywidget.h joywidget.h poswidget.h joydevice.h caldialog.h
#
