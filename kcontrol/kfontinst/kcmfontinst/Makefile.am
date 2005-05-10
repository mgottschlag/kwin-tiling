kde_module_LTLIBRARIES = kcm_fontinst.la
kcm_fontinst_la_LIBADD  = $(LIB_KIO) $(LIBFREETYPE_LIBS) $(LIBFONTCONFIG_LIBS) ../lib/libkfontinst.la
METASOURCES = AUTO
kcm_fontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
 
xdg_apps_DATA = kcmfontinst.desktop
 
kcm_fontinst_la_SOURCES = \
        KCmFontInst.cpp \
        KFileFontIconView.cpp \
        KFileFontView.cpp \
        SettingsDialog.cpp

EXTRA_DIST = \
	$(xdg_apps_DATA) \
	KCmFontInst.h \
        KFileFontIconView.h \
        KFileFontView.h \
        SettingsDialog.h

noinst_HEADERS = $(EXTRA_DIST)

KDE_ICON = AUTO
