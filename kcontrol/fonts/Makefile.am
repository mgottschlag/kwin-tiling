# $Id$

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_fonts.la

kcm_fonts_la_SOURCES = fonts.cpp
kcm_fonts_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_fonts_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/kcontrol/krdb/libkrdb.la
kcm_fonts_la_METASOURCES = fonts.moc

noinst_HEADERS = fonts.h

messages:
	$(XGETTEXT) -kni18n $(kcm_fonts_la_SOURCES) -o $(podir)/kcmfonts.pot

lnf_DATA = fonts.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(lnf_DATA)
