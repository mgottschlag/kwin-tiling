bin_PROGRAMS = kfontinst
kfontinst_SOURCES = \
Main.cpp

noinst_HEADERS=

kfontinst_LDADD = $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_KDECORE) ../lib/libkfontinst.la
kfontinst_LDFLAGS = $(all_libraries) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH)
INCLUDES= $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes)
