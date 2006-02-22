kde_module_LTLIBRARIES = kcm_clock.la

kcm_clock_la_SOURCES = dtime.cpp tzone.cpp main.cpp 

kcm_clock_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_clock_la_LIBADD = $(LIB_KIO)

AM_CPPFLAGS = $(all_includes)
 
METASOURCES = AUTO

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcmkclock.pot

xdg_apps_DATA = clock.desktop

KDE_ICON=AUTO
