# $Id$

INCLUDES = $(all_includes) $(LIBFONTCONFIG_CFLAGS)

kde_module_LTLIBRARIES = kcm_fonts.la

kcm_fonts_la_SOURCES = fonts.cpp kxftconfig.cpp
kcm_fonts_la_LDFLAGS = $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_fonts_la_LIBADD = $(LIB_KDEUI) $(LIBFONTCONFIG_LIBS)
kcm_fonts_la_METASOURCES = fonts.moc

noinst_HEADERS = fonts.h kxftconfig.h

messages:
	$(XGETTEXT) -kni18n $(kcm_fonts_la_SOURCES) -o $(podir)/kcmfonts.pot

xdg_apps_DATA = fonts.desktop
