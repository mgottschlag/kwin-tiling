LDFLAGS = $(all_libraries) $(KDE_RPATH)

bin_PROGRAMS = kpartsaver.kss
INCLUDES = -I$(top_srcdir)/kscreensaver/libkscreensaver $(all_includes)

kpartsaver_kss_SOURCES = kpartsaver.cpp configwidget.ui
kpartsaver_kss_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(top_builddir)/kscreensaver/libkscreensaver/libkscreensaver.la
kpartsaver_kss_LDADD = $(LIB_KDEUI) $(LIB_KFILE)
kpartsaver_kss_METASOURCES = AUTO

METASOURCES = AUTO

desktop_DATA = kpartsaver.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kpartsaver.pot
