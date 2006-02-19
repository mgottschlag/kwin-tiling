AM_CPPFLAGS  = $(all_includes)

kde_module_LTLIBRARIES = kcm_icons.la

kcm_icons_la_SOURCES = iconthemes.cpp icons.cpp main.cpp
kcm_icons_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_icons_la_LIBADD = $(LIB_KIO)

METASOURCES = AUTO 

noinst_HEADERS = icons.h iconthemes.h main.h

check_PROGRAMS = testicons
testicons_SOURCES = testicons.cpp 
testicons_LDFLAGS = $(all_libraries)
testicons_LDADD = $(LIB_KDEUI) icons.lo

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmicons.pot

xdg_apps_DATA = icons.desktop

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/Themes
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(DESTDIR)$(kde_appsdir)/Settings/LookNFeel/Themes/iconthemes.desktop

KDE_ICON=AUTO
