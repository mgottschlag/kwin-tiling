INCLUDES = $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kmenuedit.la

CLEANFILES = dummy.cpp

kmenuedit_la_SOURCES = basictab.cpp treeview.cpp desktopfileeditor.cpp menueditview.cpp kmenuedit.cpp main.cpp khotkeys.cpp menufile.cpp

kmenuedit_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO)
kmenuedit_la_LDFLAGS = $(all_libraries) -module -avoid-version

noinst_HEADERS = kmenuedit.h menueditview.h treeview.h desktopfileeditor.h basictab.h khotkeys.h

METASOURCES = AUTO

xdg_apps_DATA = kmenuedit.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/System
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/System/kmenuedit.desktop

KDE_ICON = kmenuedit

EXTRA_DIST = $(xdg_apps_DATA)

rcdir = $(kde_datadir)/kmenuedit
rc_DATA = kmenueditui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp -o $(podir)/kmenuedit.pot

dummy.cpp:
	echo > dummy.cpp

SUBDIRS = pixmaps
