INCLUDES= $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_accessibility.la

kcm_accessibility_la_SOURCES = accessibility.cpp 
kcm_accessibility_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_accessibility_la_LIBADD = $(LIB_KIO)

noinst_HEADERS = accessibility.h

data_DATA = accessibility.desktop
datadir = $(kde_appsdir)/Settings/Accessibility


messages:
	$(XGETTEXT) $(kcm_accessibility_la_SOURCES) -o $(podir)/kcmaccessibility.pot
