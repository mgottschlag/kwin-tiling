kde_module_LTLIBRARIES = kcm_display.la

kcm_display_la_SOURCES = display.cpp

kcm_display_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_display_la_LIBADD = $(LIB_KUTILS)
INCLUDES= $(all_includes) 

kcm_display_la_METASOURCES = AUTO

noinst_HEADERS = display.h

xdg_apps_DATA = display.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmdisplay.pot
