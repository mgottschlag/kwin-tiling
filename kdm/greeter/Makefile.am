PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = -I.. $(all_includes) $(KERBEROS_INCS)

DBGFLAGS = -g

AM_CFLAGS = $(DBGFLAGS) -DGREETER

#AM_CFLAGS   = $(AM_CFLAGS) -Wall -Wno-implicit -Wno-return-type

AM_CPPFLAGS = $(DBGFLAGS) -DGREETER -DQT_NO_CAST_ASCII

lib_LTLIBRARIES = libKdmGreet.la

libKdmGreet_la_SOURCES = kdmconfig.cpp \
	kdmshutdown.cpp \
	kfdialog.cpp \
	kdmclock.cpp \
	kgreeter.cpp \
	liloinfo.cpp \
	verify.c

# Libraries:
libKdmGreet_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KRB_RPATH) -module -avoid-version -no-undefined
libKdmGreet_la_LIBADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) \
	$(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
	$(LIBCRYPT) $(LIBUCB) $(LIBUTIL)

METASOURCES = kdmshutdown.moc kdmclock.moc kgreeter.moc liloinfo.moc kfdialog.moc

messages:
	$(XGETTEXT) $(libKdmGreet_la_SOURCES) -o $(podir)/kdmgreet.pot

noinst_HEADERS = kdmshutdown.h kdmclock.h kgreeter.h liloinfo.h kfdialog.h kdmconfig.h

