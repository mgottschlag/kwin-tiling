# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = -I$(srcdir)/.. $(all_includes) $(KERBEROS_INCS)

AM_CFLAGS = $(STD_DEFINES) $(CONN_FLAGS) -DGREETER
AM_CXXFLAGS = $(AM_CFLAGS)

bin_PROGRAMS = kdm_getcfg
kdm_getcfg_SOURCES = kdm_getcfg.cpp
kdm_getcfg_LDFLAGS = $(all_libraries)
kdm_getcfg_LDADD = $(LIB_QT) $(LIB_KDECORE)

# static
noinst_LIBRARIES = @kdmgreetera@

EXTRA_LIBRARIES = libKdmGreet.a

libKdmGreet_a_SOURCES = kdmconfig.cpp \
    kdmshutdown.cpp \
    kfdialog.cpp \
    kdmclock.cpp \
    kgreeter.cpp \
    liloinfo.cpp \
    miscfunc.c \
    verify.c

# dynamic
lib_LTLIBRARIES = @kdmgreeterla@

EXTRA_LTLIBRARIES = libKdmGreet.la

libKdmGreet_la_SOURCES = $(libKdmGreet_a_SOURCES)

libKdmGreet_la_LDFLAGS = -rpath $(libdir) $(all_libraries) $(KDE_RPATH) $(KRB_RPATH) -module -avoid-version -no-undefined
libKdmGreet_la_LIBADD  = $(LIB_KDEUI) -lXau $(PASSLIB) \
    $(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
    $(LIBCRYPT) $(LIBUCB) $(LIBUTIL)

METASOURCES = kdmshutdown.moc kdmclock.moc kgreeter.moc liloinfo.moc kfdialog.moc

messages:
	$(XGETTEXT) $(libKdmGreet_a_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h miscfunc.h

install-data-local:
	@if test ! -f $(DESTDIR)$(kde_confdir)/kdmrc; then \
	    $(mkinstalldirs) $(DESTDIR)$(kde_confdir); \
	    echo $(INSTALL_DATA) $(srcdir)/kdmrc $(DESTDIR)$(kde_confdir)/kdmrc; \
	    $(INSTALL_DATA) $(srcdir)/kdmrc $(DESTDIR)$(kde_confdir)/kdmrc; \
	fi

EXTRA_DIST = kdmrc
