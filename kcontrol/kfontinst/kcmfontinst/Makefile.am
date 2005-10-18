if include_fontinst_tn
FONTINST_PRINT_INC=-I$(srcdir)/../viewpart
FONTINST_PRINT_LIB=../viewpart/libkfontinstprint.la
endif

kde_module_LTLIBRARIES = kcm_fontinst.la
kcm_fontinst_la_LIBADD  = $(LIB_KIO) $(FONTINST_PRINT_LIB) $(LIBFREETYPE_LIBS) $(LIBFONTCONFIG_LIBS) ../lib/libkfontinst.la $(LIB_KDE3SUPPORT)
METASOURCES = AUTO
kcm_fontinst_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
AM_CPPFLAGS = -I$(srcdir)/../lib $(FONTINST_PRINT_INC) -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)

xdg_apps_DATA = kcmfontinst.desktop

kcm_fontinst_la_SOURCES = \
        KCmFontInst.cpp \
        KFileFontIconView.cpp \
        KFileFontView.cpp \
	PrintDialog.cpp \
        SettingsDialog.cpp

EXTRA_DIST = \
	$(xdg_apps_DATA) \
	KCmFontInst.h \
        KFileFontIconView.h \
        KFileFontView.h \
	PrintDialog.h \
        SettingsDialog.h

noinst_HEADERS = $(EXTRA_DIST)

KDE_ICON = AUTO
