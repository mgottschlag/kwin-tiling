kde_module_LTLIBRARIES = kcm_clock.la

kcm_clock_la_SOURCES = dtime.cpp tzone.cpp main.cpp 

kcm_clock_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_clock_la_LIBADD = -lkdeui

INCLUDES= $(all_includes)
 
kcm_clock_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_clock_la_SOURCES) -o $(podir)/kcmkclock.pot

data_DATA = clock.desktop
datadir = $(kde_appsdir)/Settings/System
