AM_CPPFLAGS  = $(all_includes)

kde_module_LTLIBRARIES = kcm_xinerama.la

kcm_xinerama_la_SOURCES = kcmxinerama.cpp xineramawidget.ui

kcm_xinerama_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_xinerama_la_LIBADD = -lkdeui

METASOURCES = AUTO 

noinst_HEADERS = kcmxinerama.h


xinerama_datadir = $(kde_appsdir)/.hidden
xinerama_data_DATA = xinerama.desktop
