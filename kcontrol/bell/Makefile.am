kde_module_LTLIBRARIES = kcm_bell.la

kcm_bell_la_SOURCES = bell.cpp

kcm_bell_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_bell_la_LIBADD = -lkdeui

AM_CPPFLAGS= $(all_includes)

METASOURCES = bell.moc

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmbell.pot

xdg_apps_DATA = bell.desktop

KDE_ICON=AUTO
