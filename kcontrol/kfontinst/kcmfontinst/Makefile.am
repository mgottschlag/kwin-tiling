kde_module_LTLIBRARIES = kcm_fontinst.la
kcm_fontinst_la_LIBADD  = $(LIB_KIO) $(LIBFREETYPE_LIBS) $(LIBFONTCONFIG_LIBS) ../lib/libkfontinst.la
kcm_fontinst_la_METASOURCES = AUTO
kcm_fontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
INCLUDES = $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -I../lib -I../kio -I../../fonts
 
desktop_DATA = kcmfontinst.desktop
desktopdir = $(kde_appsdir)/Settings/System/
 
kcm_fontinst_la_SOURCES = \
        KCmFontInst.cpp \
        KFileFontView.cpp \
        RenameJob.cpp

EXTRA_DIST = \
	$(desktop_DATA) \
	KCmFontInst.h \
        KFileFontView.h \
        RenameJob.h

noinst_HEADERS = $(EXTRA_DIST)

KDE_ICON = kcmfontinst

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmfontinst.pot
