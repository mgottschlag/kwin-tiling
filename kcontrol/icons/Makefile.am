# $Id$

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_icons.la

kcm_icons_la_SOURCES = iconthemes.cpp icons.cpp main.cpp
kcm_icons_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_icons_la_LIBADD = $(LIB_KFILE)
kcm_icons_la_METASOURCES = icons.moc iconthemes.moc main.moc


noinst_HEADERS = icons.h iconthemes.h main.h

check_PROGRAMS = testicons
testicons_SOURCES = testicons.cpp 
testicons_LDFLAGS = $(all_libraries)
testicons_LDADD = $(LIB_KDEUI) icons.lo

messages:
	$(XGETTEXT) -kni18n $(kcm_icons_la_SOURCES) -o $(podir)/kcmicons.pot

lnf_DATA = icons.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel/

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(kde_appsdir)/Settings/LookNFeel/Themes
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(kde_appsdir)/Settings/LookNFeel/Themes/iconthemes.desktop

EXTRA_DIST = $(lnf_DATA)
