####### Fiddle here

AM_CPPFLAGS = -DQT_CLEAN_NAMESPACE -DQT_NO_COMPAT

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD   =       $(LIB_KDECORE) $(LIB_KDEUI)

#######	Files

bin_PROGRAMS	= ksystrayproxy
METASOURCES	= ksystrayproxy.moc
noinst_HEADERS = ksystrayproxy.h

ksystrayproxy_SOURCES	= ksystrayproxy.cpp main.cpp

messages:
	$(XGETTEXT) $(ksystrayproxy_SOURCES) -o $(podir)/ksystrayproxy.pot


