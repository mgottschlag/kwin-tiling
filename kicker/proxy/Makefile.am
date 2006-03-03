INCLUDES = -I$(srcdir)/../core -I$(srcdir)/../libkicker -I../libkicker  $(all_includes)

bin_PROGRAMS = 
noinst_PROGRAMS = extensiondebugger
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = appletproxy.la

METASOURCES = AUTO

appletproxy_la_LIBADD = $(LIB_KDEUI) ../libkicker/libkickermain.la
appletproxy_la_SOURCES = appletproxy.cpp
appletproxy_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version


extensiondebugger_SOURCES = extensiondebugger.cpp
extensiondebugger_LDADD = ../libkicker/libkickermain.la
extensiondebugger_LDFLAGS = $(all_libraries) $(KDE_RPATH)


messages:
	$(XGETTEXT) appletproxy.cpp -o $(podir)/appletproxy.pot
	$(XGETTEXT) extensiondebugger.cpp -o $(podir)/extensionproxy.pot
