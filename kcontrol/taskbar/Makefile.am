kde_module_LTLIBRARIES = kcm_taskbar.la

kcm_taskbar_la_SOURCES = kcmtaskbarui.ui kcmtaskbar.cpp
kcm_taskbar_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_taskbar_la_LIBADD = $(top_builddir)/kicker/taskbar/libtaskbar.la -lkdeui
AM_CPPFLAGS = -I$(top_builddir)/kicker/taskbar $(all_includes) 

METASOURCES = AUTO

noinst_HEADERS = kcmtaskbar.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmtaskbar.pot

xdg_apps_DATA = kcmtaskbar.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/kcmtaskbar.desktop

kcmtaskbar.lo: ../../kicker/taskbar/taskbarsettings.h
