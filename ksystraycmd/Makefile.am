####### Fiddle here

INCLUDES = $(all_includes)
LDFLAGS  = $(all_libraries) $(KDE_RPATH)
LDADD    = $(LIB_KDECORE) $(LIB_KDEUI)

#######	Files

bin_PROGRAMS	= ksystraycmd
METASOURCES	= ksystraycmd.moc
noinst_HEADERS  = ksystraycmd.h

ksystraycmd_SOURCES = ksystraycmd.cpp main.cpp

messages:
	$(XGETTEXT) $(ksystraycmd_SOURCES) -o $(podir)/ksystraycmd.pot


