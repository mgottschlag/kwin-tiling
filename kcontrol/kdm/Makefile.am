lib_LTLIBRARIES = libkcm_kdm.la

libkcm_kdm_la_SOURCES = kdm-appear.cpp kdm-font.cpp kdm-sess.cpp \
	kdropsite.cpp utils.cpp backgnd.cpp kdm-lilo.cpp kdm-users.cpp \
 	klangcombo.cpp main.cpp bgdialogs.cpp

libkcm_kdm_la_LDFLAGS  = $(all_libraries) -module -avoid-version 
libkcm_kdm_la_LIBADD = -lkdeui -lkio -lkfile $(top_builddir)/kdesktop/libbgnd.la -lkimgio

INCLUDES= $(all_includes) -I$(top_srcdir)/kdesktop

libkcm_kdm_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(libkcm_kdm_la_SOURCES) -o $(podir)/kdmconfig.pot
