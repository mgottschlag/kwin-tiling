lib_LTLIBRARIES = libkcm_kdm.la

# ktagcombobox is first for --enable-final
libkcm_kdm_la_SOURCES = ktagcombobox.cpp \
	kdm-appear.cpp kdm-font.cpp kdm-sess.cpp \
	kdropsite.cpp utils.cpp backgnd.cpp kdm-lilo.cpp kdm-users.cpp \
 	klangcombo.cpp main.cpp bgdialogs.cpp

libkcm_kdm_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kdm_la_LIBADD = -lkdeui -lkio -lkfile $(top_builddir)/kcontrol/display/libbgnd.la -lkimgio

INCLUDES= $(all_includes) -I$(top_srcdir)/kcontrol/display -I$(top_srcdir)/kdesktop

libkcm_kdm_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcm_kdm_la_SOURCES) -o $(podir)/kdmconfig.pot

data_DATA = kdm.desktop
datadir = $(kde_appsdir)/Settings/Applications/System
