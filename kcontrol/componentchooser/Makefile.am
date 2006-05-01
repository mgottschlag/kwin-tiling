SUBDIRS= . componentservices
AM_CPPFLAGS  = -I../ -I$(top_srcdir)/kio -I$(top_srcdir) $(all_includes)

METASOURCES = AUTO 

kde_module_LTLIBRARIES = kcm_componentchooser.la

kcm_componentchooser_la_SOURCES = componentchooser_ui.ui componentconfig_ui.ui emailclientconfig_ui.ui terminalemulatorconfig_ui.ui browserconfig_ui.ui componentchooser.cpp kcm_componentchooser.cpp
kcm_componentchooser_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_componentchooser_la_LIBADD =  $(LIB_KIO)

xdg_apps_DATA = componentchooser.desktop

