INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_xinerama.la

kcm_xinerama_la_SOURCES = kcmxinerama.cpp xineramawidget.ui

kcm_xinerama_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_xinerama_la_LIBADD = -lkdeui

kcm_xinerama_la_METASOURCES = AUTO

noinst_HEADERS = kcmxinerama.h

KDE_ICON = xinerama

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmxinerama.pot

data_DATA = xinerama.desktop
datadir = $(kde_appsdir)/Settings/Desktop

