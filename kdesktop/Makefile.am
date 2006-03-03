## Makefile.am of kdebase/kdesktop

INCLUDES = -I$(top_srcdir)/workspace/lib -I$(top_srcdir)/workspace/kcontrol/background -I$(top_srcdir)/libkonq -I$(top_srcdir)/workspace/kdmlib $(all_includes)
kdesktop_la_LDFLAGS  = $(all_libraries) -module $(KDE_PLUGIN)
kdesktop_la_LIBADD   = libkdesktopsettings.la $(top_builddir)/libkonq/libkonq.la $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(top_builddir)/workspace/kdmlib/libdmctl.la $(LIB_XSS) $(top_builddir)/workspace/lib/libkworkspace.la 

SUBDIRS = . lock patterns programs init kwebdesktop

####### Files

bin_PROGRAMS = kcheckrunning
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kdesktop.la
noinst_LTLIBRARIES = libkdesktopsettings.la

libkdesktopsettings_la_LDFLAGS = $(all_libraries) -no-undefined
libkdesktopsettings_la_LIBADD = $(LIB_KDECORE)
libkdesktopsettings_la_SOURCES = kdesktopsettings.kcfgc klaunchsettings.kcfgc 

kdesktop_la_SOURCES = main.cc krootwm.cc xautolock.cc kdiconview.cc desktop.cc \
	lockeng.cc KDesktopIface.skel \
	bgmanager.cc init.cc KScreensaverIface.skel \
	minicli.cpp KBackgroundIface.skel pixmapserver.cc kcustommenu.cc \
	startupid.cpp minicli_ui.ui xautolock_diy.c xautolock_engine.c \
	kshadowengine.cpp kshadowsettings.cpp \
	kdesktopshadowsettings.cpp kfileividesktop.cpp

include_HEADERS = KDesktopIface.h KScreensaverIface.h KBackgroundIface.h

noinst_HEADERS = desktop.h bgmanager.h krootwm.h \
	xautolock.h lockeng.h init.h minicli.h \
	pixmapserver.h startupid.h xautolock_c.h

kcheckrunning_SOURCES = kcheckrunning.cpp
kcheckrunning_LDFLAGS = $(all_libraries)
kcheckrunning_LDADD = $(LIB_X11)

METASOURCES = AUTO

####### Build rules

# List made with: grep -l kdesktopsettings.h *[pc] | sed -e 's/\.c.*$/.lo: kdesktopsettings.h/'
bgmanager.lo: kdesktopsettings.h
desktop.lo: kdesktopsettings.h
init.lo: kdesktopsettings.h
kdesktopsettings.lo: kdesktopsettings.h
kdiconview.lo: kdesktopsettings.h
krootwm.lo: kdesktopsettings.h
lockeng.lo: kdesktopsettings.h
main.lo: kdesktopsettings.h
minicli.lo: kdesktopsettings.h

# List made with: grep -l klaunchsettings.h *[pc] | sed -e 's/\.c.*$/.lo: klaunchsettings.h/'
desktop.lo: klaunchsettings.h
klaunchsettings.lo: klaunchsettings.h
startupid.lo: klaunchsettings.h

messages: rc.cpp
	$(EXTRACTRC) lock/*.ui >> rc.cpp
	$(XGETTEXT) lock/*.cc *.cc *.cpp *.h -o $(podir)/kdesktop.pot

menu_DATA = kdesktop_custom_menu1 kdesktop_custom_menu2
menudir = $(kde_confdir)

autostart_DATA = kdesktop.desktop
autostartdir = $(datadir)/autostart

konqservice_DATA = kdesktopSetAsBackground.desktop
konqservicedir = $(kde_datadir)/konqueror/servicemenus

kde_kcfg_DATA = kdesktop.kcfg klaunch.kcfg
