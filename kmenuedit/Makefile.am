INCLUDES = $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kmenuedit.la kcontroledit.la

noinst_LTLIBRARIES = libkmenueditcommon.la

CLEANFILES = dummy.cpp

libkmenueditcommon_la_SOURCES = basictab.cpp treeview.cpp kmenuedit.cpp \
	khotkeys.cpp menufile.cpp menuinfo.cpp

libkmenueditcommon_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDE3SUPPORT)
libkmenueditcommon_la_LDFLAGS = $(all_libraries) -module -avoid-version

kmenuedit_la_SOURCES = main.cpp
kmenuedit_la_LIBADD = libkmenueditcommon.la
kmenuedit_la_LDFLAGS = $(all_libraries) -module -avoid-version

kcontroledit_la_SOURCES = kcontrol_main.cpp
kcontroledit_la_LIBADD = libkmenueditcommon.la
kcontroledit_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = kmenuedit.h treeview.h basictab.h khotkeys.h \
	menufile.h menuinfo.h

METASOURCES = AUTO

xdg_apps_DATA = kmenuedit.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/System
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/System/kmenuedit.desktop

KDE_ICON = kmenuedit

EXTRA_DIST = $(xdg_apps_DATA)

rcdir = $(kde_datadir)/kmenuedit
rc_DATA = kmenueditui.rc

rc2dir = $(kde_datadir)/kcontroledit
rc2_DATA = kcontroleditui.rc


dummy.cpp:
	echo > dummy.cpp

SUBDIRS = pixmaps
