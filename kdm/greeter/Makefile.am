# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = -I$(srcdir)/.. $(all_includes) $(KERBEROS_INCS)

DBGFLAGS = -g
AM_CFLAGS = $(STD_DEFINES) $(CONN_FLAGS) $(DBGFLAGS) -DGREETER
AM_CPPFLAGS = $(AM_CFLAGS) -DQT_NO_CAST_ASCII

# static
noinst_LIBRARIES = @kdmgreetera@

EXTRA_LIBRARIES = libKdmGreet.a

libKdmGreet_a_SOURCES = kdmconfig.cpp \
    kdmshutdown.cpp \
    kfdialog.cpp \
    kdmclock.cpp \
    kgreeter.cpp \
    liloinfo.cpp \
    verify.c

# dynamic
lib_LTLIBRARIES = @kdmgreeterla@

EXTRA_LTLIBRARIES = libKdmGreet.la

libKdmGreet_la_SOURCES = $(libKdmGreet_a_SOURCES)

libKdmGreet_la_LDFLAGS = -rpath $(libdir) $(all_libraries) $(KDE_RPATH) $(KRB_RPATH) -module -avoid-version -no-undefined
libKdmGreet_la_LIBADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) \
    $(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
    $(LIBCRYPT) $(LIBUCB) $(LIBUTIL)

METASOURCES = kdmshutdown.moc kdmclock.moc kgreeter.moc liloinfo.moc kfdialog.moc

messages:
	$(XGETTEXT) $(libKdmGreet_a_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h

install-data-local:
	@if test ! -f $(DESTDIR)$(kde_confdir)/kdmrc; then \
	    $(mkinstalldirs) $(DESTDIR)$(kde_confdir); \
	    echo $(INSTALL_DATA) $(srcdir)/kdmrc $(DESTDIR)$(kde_confdir)/kdmrc; \
	    $(INSTALL_DATA) $(srcdir)/kdmrc $(DESTDIR)$(kde_confdir)/kdmrc; \
	fi

EXTRA_DIST = kdmrc
