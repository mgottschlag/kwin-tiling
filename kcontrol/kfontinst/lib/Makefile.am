noinst_LTLIBRARIES = libkfontinst.la

libkfontinst_la_SOURCES = \
CompressedFile.cpp \
Config.cpp \
Encodings.cpp \
FontEngine.cpp \
Fontmap.cpp \
GetPid.c \
Global.cpp \
Misc.cpp \
XConfig.cpp

noinst_HEADERS = \
CompressedFile.h \
Config.h \
Encodings.h \
FontEngine.h \
Fontmap.h \
Global.h \
Misc.h \
XConfig.h

libkfontinst_la_LIBADD = $(LIB_FONT_ENC) $(LIB_KIO) ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS)
libkfontinst_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -no-undefined
INCLUDES= $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -DOS_$(UNAME) -I$(srcdir)/../../fonts $(all_includes)
