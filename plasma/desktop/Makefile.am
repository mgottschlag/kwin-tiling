## Makefile.am of Plasma Desktop

INCLUDES = -I$(top_srcdir)/workspace/plasma/lib $(all_includes)

bin_PROGRAMS = pdesktop
pdesktop_LDFLAGS  = $(all_libraries) $(KDE_RPATH)
pdesktop_LDADD    = ../lib/libplasma.la $(LIB_KIO) $(LIB_QTOPENGL)

####### Files

pdesktop_SOURCES = desktop.cpp main.cpp

METASOURCES = AUTO
