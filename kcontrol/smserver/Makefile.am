kde_module_LTLIBRARIES = kcm_smserver.la

kcm_smserver_la_SOURCES = kcmsmserver.cpp smserverconfigdlg.ui smserverconfigimpl.cpp
kcm_smserver_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_smserver_la_LIBADD = -lkdeui
INCLUDES= $(all_includes) 

kcm_smserver_la_METASOURCES = AUTO

noinst_HEADERS = kcmsmserver.h smserverconfigimpl.h

KDE_ICON = go

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmsmserver.pot

xdg_apps_DATA = kcmsmserver.desktop

EXTRA_DIST = $(xdg_apps_DATA)
