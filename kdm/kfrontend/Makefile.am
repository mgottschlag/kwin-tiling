# use 'make GENKDMCONF_FLAGS=... install' to override
GENKDMCONF_FLAGS =

SUBDIRS = themer themes pics sessions

AM_CPPFLAGS = -I$(srcdir)/../backend -I.. -I$(top_srcdir)/workspace/kcontrol/background \
    -I$(top_srcdir)/kdmlib $(all_includes)

bin_PROGRAMS = kdm_config kdm_greet krootimage genkdmconf kdmctl

kdm_config_SOURCES = kdm_config.c
kdm_config_LDADD = $(LIBRESOLV) $(LIBSOCKET) $(LIBPOSIX4)

kdm_greet_SOURCES = \
    kdm_greet.c \
    kdmconfig.cpp \
    kdmclock.cpp \
    kconsole.cpp \
    kfdialog.cpp \
    kgdialog.cpp \
    kchooser.cpp \
    kgverify.cpp \
    kdmshutdown.cpp \
    kgreeter.cpp \
    kgapp.cpp
kdm_greet_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kdm_greet_LDADD = themer/libkdmthemer.a $(LIB_KDEUI) $(XTESTLIB) $(LIBPOSIX4)

krootimage_SOURCES = krootimage.cpp
krootimage_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krootimage_LDADD = $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(LIB_KIO)

METASOURCES = AUTO

genkdmconf_SOURCES = genkdmconf.c
genkdmconf_LDFLAGS = $(X_LDFLAGS) $(X_RPATH)
genkdmconf_LDADD = $(LIB_X11)

kdmctl_SOURCES = kdmctl.c
kdmctl_LDADD = $(LIBSOCKET)

install-data-local: genkdmconf
	./genkdmconf --in $(DESTDIR)$(kde_confdir)/kdm --face-src $(srcdir)/pics $(GENKDMCONF_FLAGS)

messages:
	$(XGETTEXT) `find . -name "*.cpp"` -o $(podir)/kdmgreet.pot

noinst_HEADERS = \
    kdm_greet.h \
    kdmconfig.h \
    kdmclock.h \
    kconsole.h \
    kfdialog.h \
    kgdialog.h \
    kchooser.h \
    kgverify.h \
    kdmshutdown.h \
    kgreeter.h \
    kgapp.h \
    \
    krootimage.h 

kdm_greet_COMPILE_FIRST = ../config.ci
kdm_config_COMPILE_FIRST = ../config.ci
genkdmconf_COMPILE_FIRST = ../config.ci

