
AM_CPPFLAGS  = $(all_includes)
SUBDIRS = keramik . 

kde_module_LTLIBRARIES = kcm_style.la

kcm_style_la_SOURCES = kcmstyle.cpp stylepreview.ui menupreview.cpp styleconfdialog.cpp
kcm_style_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_style_la_LIBADD = $(LIB_KIO) ../krdb/libkrdb.la
METASOURCES = AUTO 

noinst_HEADERS = kcmstyle.h stylepreview.h menupreview.h

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmstyle.pot

xdg_apps_DATA = style.desktop

KDE_ICON=AUTO
