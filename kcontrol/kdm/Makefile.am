kde_module_LTLIBRARIES = kcm_kdm.la

# _don't_ add a theme configurator!!
kcm_kdm_la_SOURCES = background.cpp kdm-appear.cpp kdm-font.cpp kdm-shut.cpp \
	kdm-users.cpp kdm-conv.cpp main.cpp kbackedcombobox.cpp

kcm_kdm_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) -module -avoid-version -no-undefined
kcm_kdm_la_LIBADD = $(top_builddir)/workspace/kcontrol/background/libkcmbgnd.la $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(LIB_KIO) $(LIB_KNEWSTUFF) -lkde3support

AM_CPPFLAGS= -I$(top_builddir)/workspace/kcontrol/background \
          -I$(top_srcdir)/workspace/kcontrol/background \
          -I$(top_srcdir)/workspace/kdesktop $(all_includes)

METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_kdm_la_SOURCES) -o $(podir)/kdmconfig.pot

xdg_apps_DATA = kdm.desktop
KDE_ICON=AUTO
