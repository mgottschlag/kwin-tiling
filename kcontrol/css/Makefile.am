INCLUDES = $(all_includes)

template_DATA = template.css
templatedir = $(kde_datadir)/kcmcss

kde_module_LTLIBRARIES = kcm_css.la
kcm_css_la_SOURCES = template.cpp cssconfig.ui kcmcss.cpp preview.ui
kcm_css_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_css_la_LIBADD = -lkdeui $(LIB_KFILE)

kcm_css_la_METASOURCES = AUTO
 
messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmcss.pot
 
data_DATA = kcmcss.desktop
datadir = $(kde_appsdir)/Settings/WebBrowsing
