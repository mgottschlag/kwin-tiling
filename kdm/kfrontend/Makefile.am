# use 'make GENKDMCONF_FLAGS=... install' to override
GENKDMCONF_FLAGS =

SUBDIRS = pics sessions

AM_CPPFLAGS = -I$(srcdir)/../backend -I$(top_srcdir)/kcontrol/background \
    $(all_includes) $(XDM_DEFINES)

bin_PROGRAMS = kdm_config kdm_greet krootimage genkdmconf kdmctl

kdm_config_SOURCES = kdm_config.c
kdm_config_LDADD = $(LIBRESOLV) $(LIBSOCKET) $(LIBPOSIX4)

kdm_greet_SOURCES = \
    kdm_greet.c \
    liloinfo.cpp \
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
kdm_greet_LDADD = $(LIB_KDEUI) $(XTESTLIB) $(LIBPOSIX4)

krootimage_SOURCES = krootimage.cpp
krootimage_LDFLAGS = $(all_libraries) $(KDE_RPATH)
krootimage_LDADD = $(top_builddir)/kcontrol/background/libbgnd.la $(LIB_KIO)

kde_module_LTLIBRARIES = kgreet_classic.la kgreet_winbind.la

kgreet_classic_la_SOURCES = kgreet_classic.cpp
kgreet_classic_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kgreet_classic_la_LIBADD = $(LIB_KDEUI)

kgreet_winbind_la_SOURCES = kgreet_winbind.cpp
kgreet_winbind_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kgreet_winbind_la_LIBADD = $(LIB_KDEUI)

METASOURCES = AUTO

genkdmconf_SOURCES = genkdmconf.c
genkdmconf_LDFLAGS = $(X_LDFLAGS) $(X_RPATH)
genkdmconf_LDADD = $(LIB_X11)

kdmctl_SOURCES = kdmctl.c
kdmctl_LDADD = $(LIBSOCKET)

install-data-local: genkdmconf
	./genkdmconf --in $(DESTDIR)$(kde_confdir)/kdm --face-src $(srcdir)/pics $(GENKDMCONF_FLAGS)

messages:
	$(XGETTEXT) $(kdm_greet_SOURCES) -o $(podir)/kdmgreet.pot
	$(XGETTEXT) $(kgreet_classic_la_SOURCES) -o $(podir)/kgreet_classic.pot

noinst_HEADERS = \
    kdm_greet.h \
    liloinfo.h \
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
    kgreet_classic.h \
    kgreet_winbind.h \
    \
    krootimage.h 

include_HEADERS = kgreeterplugin.h

kdm_greet_COMPILE_FIRST = config.ci
kdm_config_COMPILE_FIRST = config.ci
genkdmconf_COMPILE_FIRST = config.ci

config.ci: $(srcdir)/config.def $(srcdir)/confproc.pl
	$(PERL) -w $(srcdir)/confproc.pl $(srcdir)/config.def $@

CLEANFILES = config.ci
