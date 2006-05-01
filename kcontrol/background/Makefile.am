SUBDIRS = . pics

AM_CPPFLAGS = -I$(top_srcdir)/libkonq $(all_includes)

kde_module_LTLIBRARIES = kcm_background.la

kcm_background_la_SOURCES = main.cpp
kcm_background_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_background_la_LIBADD = libkcmbgnd.la libbgnd.la $(LIB_KFILE) $(LIB_KNEWSTUFF) $(LIB_KDE3SUPPORT)

METASOURCES = AUTO

noinst_HEADERS = main.h \
 bgrender.h bgsettings.h \
 bgdialog.h bgadvanced.h bgwallpaper.h bgdefaults.h bgmonitor.h

noinst_LTLIBRARIES = libbgnd.la libkcmbgnd.la
libbgnd_la_SOURCES = bgrender.cpp bgsettings.cpp
libkcmbgnd_la_SOURCES = bgdialog.cpp bgdialog_ui.ui bgwallpaper_ui.ui \
                bgwallpaper.cpp bgadvanced_ui.ui bgadvanced.cpp bgmonitor.cpp

main.lo: bgdialog_ui.h


xdg_apps_DATA = background.desktop

KDE_ICON=AUTO
