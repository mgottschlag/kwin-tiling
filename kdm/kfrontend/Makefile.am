#XCONSOLE_SRC = kconsole.cpp
#XCONSOLE_CPPFLAGS = -DBUILTIN_XCONSOLE
#XCONSOLE_LDADD = $(LIBUTIL)

SUBDIRS = pics sessions

INCLUDES = -I$(srcdir)/../backend -I$(top_srcdir)/kcontrol/background $(all_includes)

bin_PROGRAMS = kdm_config kdm_greet krootimage genkdmconf

kdm_config_SOURCES = kdm_config.c
kdm_config_LDADD = $(LIBRESOLV) $(LIBSOCKET)

AM_CPPFLAGS = $(XCONSOLE_CPPFLAGS)
EXTRA_DIST = kconsole.cpp

kdm_greet_SOURCES = \
    kdm_greet.c \
    liloinfo.cpp \
    kdmconfig.cpp \
    kdmshutdown.cpp \
    kfdialog.cpp \
    kdmclock.cpp \
    $(XCONSOLE_SRC) \
    kchooser.cpp \
    kgreeter.cpp
kdm_greet_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdm_greet_LDADD = $(LIB_KDEUI) $(XTESTLIB) $(XCONSOLE_LDADD)

krootimage_SOURCES = krootimage.cpp
krootimage_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krootimage_LDADD = $(top_builddir)/kcontrol/background/libbgnd.la $(LIB_KIO)

METASOURCES = AUTO

genkdmconf_SOURCES = genkdmconf.c
genkdmconf_LDFLAGS = $(X_LDFLAGS) $(X_RPATH)
genkdmconf_LDADD = $(LIB_X11)

install-data-local: genkdmconf
	@./genkdmconf --in $(DESTDIR)$(kde_confdir)/kdm --face-src $(SRCDIR)/pics $(GENKDMCONF_FLAGS)

messages:
	$(XGETTEXT) $(kdm_greet_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdm_greet.h kdm_config.h kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h krootimage.h kconsole.h kchooser.h

