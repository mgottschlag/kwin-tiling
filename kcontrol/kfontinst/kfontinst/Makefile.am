bin_PROGRAMS = kfontinst
kfontinst_SOURCES = \
CompressedFile.cpp \
Main.cpp \
Encodings.cpp \
FontEngine.cpp \
Fontmap.cpp \
GetPid.c \
XConfig.cpp

noinst_HEADERS= \
CompressedFile.h \
Encodings.h \
FontEngine.h \
Fontmap.h \
XConfig.h

kfontinst_LDADD = ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_FONT_ENC) $(LIBZ) $(LIB_KIO) ../lib/libkfontinst.la
kfontinst_LDFLAGS = $(all_libraries) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) $(KDE_RPATH)
AM_CPPFLAGS= $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -DOS_$(UNAME) -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes)
