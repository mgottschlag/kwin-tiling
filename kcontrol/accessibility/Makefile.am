INCLUDES= $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_accessibility.la

kcm_accessibility_la_SOURCES = accessibility.cpp accessibilityconfigwidget.ui
kcm_accessibility_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_accessibility_la_LIBADD = $(LIB_KIO)

noinst_HEADERS = accessibility.h

kcmaccessibility_data_DATA = accessibility.desktop
kcmaccessibility_datadir = $(kde_appsdir)/Settings/Accessibility

messages: rc.cpp
	$(EXTRACTRC) *.ui *.rc >> rc.cpp
	$(XGETTEXT) $(kcm_accessibility_la_SOURCES) rc.cpp -o $(podir)/kcmaccessibility.pot
