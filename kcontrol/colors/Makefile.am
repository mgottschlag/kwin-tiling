# $Id$

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_colors.la

kcm_colors_la_SOURCES = colorscm.cpp widgetcanvas.cpp 
kcm_colors_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_colors_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/kcontrol/display/libkrdb.la
kcm_colors_la_METASOURCES = colorscm.moc widgetcanvas.moc

noinst_HEADERS = colorscm.h widgetcanvas.h

messages:
	$(XGETTEXT) -kni18n $(kcm_colors_la_SOURCES) -o $(podir)/kcmcolors.pot

lnf_DATA = colors.desktop
lnfdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(lnf_DATA)

