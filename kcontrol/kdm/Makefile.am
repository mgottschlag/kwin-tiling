kde_module_LTLIBRARIES = kcm_kdm.la

kcm_kdm_la_SOURCES = kdm-appear.cpp kdm-font.cpp kdm-sess.cpp \
	kdropsite.cpp backgnd.cpp kdm-users.cpp kdm-conv.cpp \
	klanguagebutton.cpp main.cpp bgdialogs.cpp

kcm_kdm_la_LDFLAGS  = $(KDE_RPATH) $(all_libraries) -module -avoid-version -no-undefined
kcm_kdm_la_LIBADD = $(top_builddir)/kcontrol/background/libbgnd.la $(LIB_KIO)

INCLUDES= -I$(top_srcdir)/kcontrol/background -I$(top_srcdir)/kdesktop $(all_includes)

kcm_kdm_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(kcm_kdm_la_SOURCES) -o $(podir)/kdmconfig.pot

data_DATA = kdm.desktop
datadir = $(kde_appsdir)/Settings/System
