# $Id$

INCLUDES = $(all_includes)

SUBDIRS = . kcs ad pics

kde_module_LTLIBRARIES = kcm_style.la

noinst_LTLIBRARIES = libkrdb.la
libkrdb_la_SOURCES = krdb.cpp
libkrdb_la_LIBADD = $(LIB_KDECORE)
libkrdb_la_LDFLAGS = -no-undefined $(all_libraries) 

kcm_style_la_SOURCES = general.cpp
kcm_style_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_style_la_LIBADD = $(LIB_KFILE) libkrdb.la
kcm_style_la_METASOURCES = general.moc

noinst_HEADERS = general.h krdb.h

messages:
	$(XGETTEXT) -kni18n $(kcm_style_la_SOURCES) -o $(podir)/kcmstyle.pot

theme_DATA = style.desktop
themedir = $(kde_appsdir)/Settings/LookNFeel

update_DATA = kcmdisplayrc.upd
updatedir = $(kde_datadir)/kconf_update

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(kde_appsdir)/Settings/LookNFeel/Themes
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(kde_appsdir)/Settings/LookNFeel/Themes/style.desktop

EXTRA_DIST = $(theme_DATA)
