lib_LTLIBRARIES = libkfontviewpart.la

libkfontviewpart_la_SOURCES = kfontviewpart.cpp kfontviewpart_factory.cpp
libkfontviewpart_la_LDFLAGS = -module $(KDE_PLUGIN)
libkfontviewpart_la_LIBADD = $(LIB_KPARTS)

noinst_HEADERS  = kfontviewpart.h kfontviewpart_factory.h

kde_services_DATA = kfontviewpart.desktop

INCLUDES = $(all_includes) -I$(srcdir)/../kfontinst
METASOURCES = AUTO

bin_SCRIPTS = kcmfontinst_install_fonts.sh
konqservice_DATA = kcmfontinst_install_fonts.desktop
konqservicedir = $(kde_datadir)/konqueror/servicemenus

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kfontviewpart.pot
