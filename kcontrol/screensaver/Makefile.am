
INCLUDES = $(all_includes)

kde_module_LTLIBRARIES = kcm_screensaver.la

kcm_screensaver_la_SOURCES = scrnsave.cpp testwin.cpp saverlist.cpp saverconfig.cpp advanceddialog.cpp
kcm_screensaver_la_LDFLAGS = -module -avoid-version $(all_libraries) -no-undefined
kcm_screensaver_la_LIBADD = $(LIB_KSYCOCA) $(DPMSLIB)
kcm_screensaver_la_METASOURCES = AUTO

noinst_HEADERS = scrnsave.h

messages:
	sed 's/X-KDE-Category=\(.*\)/i18n\("Screen saver category","\1"\);/' < category_list > category_list.cpp
	$(XGETTEXT) category_list.cpp $(kcm_screensaver_la_SOURCES) -o $(podir)/kcmscreensaver.pot
	-rm -f category_list.cpp

xdg_apps_DATA = screensaver.desktop
