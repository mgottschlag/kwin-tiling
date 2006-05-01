kde_module_LTLIBRARIES = kcm_smserver.la

kcm_smserver_la_SOURCES = kcmsmserver.cpp smserverconfigdlg.ui smserverconfigimpl.cpp
kcm_smserver_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_smserver_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la -lkdeui
AM_CPPFLAGS= -I$(top_srcdir)/workspace/lib $(all_includes) 

METASOURCES = AUTO

noinst_HEADERS = kcmsmserver.h smserverconfigimpl.h


xdg_apps_DATA = kcmsmserver.desktop

EXTRA_DIST = $(xdg_apps_DATA)
