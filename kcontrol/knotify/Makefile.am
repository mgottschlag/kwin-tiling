
kde_module_LTLIBRARIES = kcm_knotify.la

kcm_knotify_la_SOURCES = knotify.cpp playersettings.ui

kcm_knotify_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_knotify_la_LIBADD = $(LIB_KIO)  -lknotifyconfig

AM_CPPFLAGS = $(all_includes)
 
METASOURCES = AUTO 

noinst_HEADERS = knotify.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmnotify.pot

xdg_apps_DATA = kcmnotify.desktop


# the following should be gone at release time.
install-data-local:
	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/knotify.desktop
	rm -f $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/kcmnotify.desktop

KDE_ICON=AUTO

