SUBDIRS = . pics

INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_background.la

kcm_background_la_SOURCES = main.cpp
kcm_background_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
kcm_background_la_LIBADD = libkcmbgnd.la libbgnd.la $(LIB_KFILE)

METASOURCES = AUTO

noinst_LTLIBRARIES = libbgnd.la libkcmbgnd.la
libbgnd_la_SOURCES = bgrender.cpp bgsettings.cpp
libkcmbgnd_la_SOURCES = bgdialog.cpp bgdialog_ui.ui bgwallpaper_ui.ui \
                bgwallpaper.cpp bgadvanced_ui.ui bgadvanced.cpp bgmonitor.cpp


messages:
        $(XGETTEXT) -kni18n *.cc *.cpp -o $(podir)/kcmbackground.pot

desktop_DATA =  background.desktop
desktopdir = $(kde_appsdir)/Settings/LookNFeel

EXTRA_DIST = $(desktop_DATA)
