# $Id$

INCLUDES = $(all_includes)

SUBDIRS = . kcs ad pics

kde_module_LTLIBRARIES = libkcm_style.la

libkcm_style_la_SOURCES = general.cpp krdb.cpp
libkcm_style_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
libkcm_style_la_LIBADD = $(LIB_KFILE)
libkcm_style_la_METASOURCES = general.moc

noinst_HEADERS = general.h

messages:
	$(XGETTEXT) -kni18n $(libkcm_style_la_SOURCES) -o $(podir)/kcmstyle.pot

theme_DATA = style.desktop
themedir = $(kde_appsdir)/Settings/LookNFeel

update_DATA = kcmdisplayrc.upd
updatedir = $(kde_datadir)/kconf_update

install-data-local: uninstall.desktop
	$(mkinstalldirs) $(kde_appsdir)/Settings/LookNFeel/Themes
	$(INSTALL_DATA) $(srcdir)/uninstall.desktop $(kde_appsdir)/Settings/LookNFeel/Themes/style.desktop

EXTRA_DIST = $(theme_DATA)
