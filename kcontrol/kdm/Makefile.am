kde_module_LTLIBRARIES = libkcm_kdm.la

# ktagcombobox is first for --enable-final
libkcm_kdm_la_SOURCES = ktagcombobox.cpp \
	kdm-appear.cpp kdm-font.cpp kdm-sess.cpp \
	kdropsite.cpp backgnd.cpp kdm-users.cpp kdm-conv.cpp \
	klangcombo.cpp main.cpp bgdialogs.cpp

libkcm_kdm_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
libkcm_kdm_la_LIBADD = -lkdeui -lkio -lkfile $(top_builddir)/kcontrol/background/libbgnd.la

INCLUDES= -I$(top_srcdir)/kcontrol/background -I$(top_srcdir)/kdesktop $(all_includes)

libkcm_kdm_la_METASOURCES = backgnd.moc kdm-conv.moc kdm-users.moc \
	ktagcombobox.moc bgdialogs.moc kdm-font.moc kdropsite.moc main.moc \
	kdm-appear.moc kdm-sess.moc klangcombo.moc

messages:
	$(XGETTEXT) $(libkcm_kdm_la_SOURCES) -o $(podir)/kdmconfig.pot

data_DATA = kdm.desktop
datadir = $(kde_appsdir)/Settings/System
