INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = kickermenu_systemmenu.la

kickermenu_systemmenu_la_SOURCES = systemmenu.cpp
kickermenu_systemmenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
kickermenu_systemmenu_la_LIBADD = $(LIB_KDEUI) 

kickermenu_systemmenu_la_METASOURCES = AUTO

desktopmenu_DATA = systemmenu.desktop
desktopmenudir = $(kde_datadir)/kicker/menuext

