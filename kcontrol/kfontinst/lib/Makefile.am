noinst_LTLIBRARIES = libkfontinst.la

libkfontinst_la_SOURCES = \
CompressedFile.cpp \
Config.cpp \
Encodings.cpp \
FontEngine.cpp \
Fontmap.cpp \
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
XConfig.h \
GetPid.c

libkfontinst_la_LIBADD = $(LIB_FONT_ENC) $(LIB_KIO) $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS)
libkfontinst_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -no-undefined
INCLUDES= $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) $(all_includes) -DOS_$(UNAME) -I../../fonts
