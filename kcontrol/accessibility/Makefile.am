AM_CPPFLAGS = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_accessibility.la

kcm_accessibility_la_SOURCES = accessibility.cpp accessibilityconfigwidget.ui
kcm_accessibility_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_accessibility_la_LIBADD = $(LIB_KIO)

noinst_HEADERS = accessibility.h

xdg_apps_DATA = accessibility.desktop

