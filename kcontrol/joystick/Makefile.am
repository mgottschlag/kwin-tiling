AM_CPPFLAGS = $(all_includes)
METASOURCES = AUTO

# Install this plugin in the KDE modules directory
kde_module_LTLIBRARIES = kcm_joystick.la

kcm_joystick_la_SOURCES = joystick.cpp joywidget.cpp poswidget.cpp joydevice.cpp caldialog.cpp
kcm_joystick_la_LIBADD = $(LIB_KDEUI)
kcm_joystick_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined

xdg_apps_DATA = joystick.desktop

messages:
	$(XGETTEXT) *.cpp -o $(podir)/joystick.pot

noinst_HEADERS =  joywidget.h joywidget.h poswidget.h joydevice.h caldialog.h

