SUBDIRS = pics

INCLUDES = -I$(srcdir)/../backend -I$(top_srcdir)/kcontrol/background $(all_includes)

bin_PROGRAMS = kdm_config kdm_greet krootimage

kdm_config_SOURCES = kdm_config.c
kdm_config_LDADD = $(LIBRESOLV) $(LIBSOCKET)

kdm_greet_SOURCES = \
    kdm_greet.c \
    liloinfo.cpp \
    kdmconfig.cpp \
    kdmshutdown.cpp \
    kfdialog.cpp \
    kdmclock.cpp \
    kgreeter.cpp
kdm_greet_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdm_greet_LDADD = $(LIB_QT) $(LIB_KDECORE) $(XTESTLIB) $(LIBXINERAMA) $(LIB_KDEUI)

krootimage_SOURCES = krootimage.cpp
krootimage_LDFLAGS = $(KDE_RPATH) $(all_libraries)
krootimage_LDADD = $(top_builddir)/kcontrol/background/libbgnd.la $(LIB_KIO)

METASOURCES = AUTO

noinst_PROGRAMS = genkdmconf
genkdmconf_SOURCES = genkdmconf.c
genkdmconf_LDFLAGS = -L$(x_libraries)
genkdmconf_LDADD = $(LIB_X11)

install-data-local:
	@$(mkinstalldirs) $(DESTDIR)$(kde_confdir)/kdm
	@./genkdmconf --in $(DESTDIR)$(kde_confdir)/kdm

messages:
	$(XGETTEXT) $(kdm_greet_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdm_greet.h kdm_config.h kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h krootimage.h

