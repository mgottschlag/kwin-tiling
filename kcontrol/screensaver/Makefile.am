
AM_CPPFLAGS = $(GLINC) $(all_includes)

kde_module_LTLIBRARIES = kcm_screensaver.la

kcm_screensaver_la_SOURCES = advanceddialogimpl.ui scrnsave.cpp testwin.cpp saverlist.cpp \
        saverconfig.cpp advanceddialog.cpp kswidget.cpp
kcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_screensaver_la_LIBADD = $(LIB_KSYCOCA) $(DPMSLIB) $(GLLIB)
METASOURCES = AUTO

noinst_HEADERS = scrnsave.h

messages: rc.cpp
	sed 's/X-KDE-Category=\(.*\)/i18n\("Screen saver category","\1"\);/' < category_list > category_list.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmscreensaver.pot
	-rm -f category_list.cpp

xdg_apps_DATA = screensaver.desktop

KDE_ICON=AUTO
