AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

noinst_LTLIBRARIES = libkxftconfig.la

libkxftconfig_la_SOURCES = kxftconfig.cpp
libkxftconfig_la_LIBADD = $(LIBFONTCONFIG_LIBS)
libkxftconfig_la_LDFLAGS = $(LIBFONTCONFIG_RPATH)

INCLUDES = $(all_includes) $(LIBFONTCONFIG_CFLAGS)

kde_module_LTLIBRARIES = kcm_fonts.la

kcm_fonts_la_SOURCES = fonts.cpp
kcm_fonts_la_LDFLAGS = $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_fonts_la_LIBADD = libkxftconfig.la $(top_builddir)/kcontrol/krdb/libkrdb.la $(LIB_KDEUI) $(LIBFONTCONFIG_LIBS)
kcm_fonts_la_METASOURCES = fonts.moc

noinst_HEADERS = fonts.h kxftconfig.h

messages:
	$(XGETTEXT) -kni18n $(kcm_fonts_la_SOURCES) -o $(podir)/kcmfonts.pot

xdg_apps_DATA = fonts.desktop
