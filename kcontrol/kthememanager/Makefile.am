INCLUDES    = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_kthememanager.la

kcm_kthememanager_la_SOURCES = kthemedlg.ui newthemewidget.ui knewthemedlg.cpp ktheme.cpp kthememanager.cpp
kcm_kthememanager_la_LIBADD = $(LIB_KIO)
kcm_kthememanager_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module

noinst_HEADERS = kthememanager.h ktheme.h knewthemedlg.h kthemedlg.ui.h

xdg_apps_DATA = kthememanager.desktop

# MIME type
kthememime_DATA = x-ktheme.desktop
kthememimedir = $(kde_mimedir)/application

# Service menu for installing themes
konqservice_DATA = kthemeservice.desktop
konqservicedir = $(kde_datadir)/konqueror/servicemenus

bin_SCRIPTS = kdeinstallktheme

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kthememanager.pot
