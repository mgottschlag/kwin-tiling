# $Id$

AM_CPPFLAGS = -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST
SUBDIRS	= config pics $(xdmconfigsubdir)

# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes) $(KERBEROS_INCS)

#AM_CFLAGS   = -Wall -Wno-implicit -Wno-return-type

bin_PROGRAMS = kdm chooser

# Libraries:
LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KRB_RPATH)
LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) \
	$(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
	$(LIBCRYPT) $(LIBUCB) $(LIBUTIL) 

# Sources for xdm (both C and C++):
kdmlib_source = access.c \
           auth.c \
	   choose.c \
	   daemon.c \
	   dm.c \
	   dpylist.c \
	   error.c \
	   file.c \
	   genauth.c \
	   mitauth.c \
	   netaddr.c \
	   policy.c \
	   protodpy.c \
	   reset.c \
	   resource.c \
	   rpcauth.c \
	   server.c \
	   session.c \
	   socket.c \
	   streams.c \
	   util.c \
	   verify.c \
	   xdmauth.c \
	   xdmcp.c

kdm_SOURCES = kdmconfig.cpp \
	kdmshutdown.cpp \
	kfdialog.cpp \
	kdmclock.cpp \
	kgreeter.cpp \
	liloinfo.cpp \
	$(kdmlib_source)

chooser_SOURCES = \
        chooser.cpp \
	CXdmcp.cpp \
	DXdmcp.cpp \
	kfdialog.cpp

chooser_LDFLAGS = $(all_libraries) $(KDE_RPATH)
chooser_LDADD  = $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) $(LIB_LIBS) \
	$(LIBSOCKET) $(LIBCRYPT) $(LIBUCB) $(LIBUTIL)  -lXt

noinst_HEADERS = kdmconfig.h kdm-config.h kdmshutdown.h kfdialog.h dm.h kdmclock.h kgreeter.h greet.h liloinfo.h

METASOURCES = kfdialog.moc kdmshutdown.moc kdmclock.moc kgreeter.moc liloinfo.moc CXdmcp.moc DXdmcp.moc

messages:
	$(XGETTEXT) $(kdm_SOURCES) $(kdmdesktop_SOURCES) -o $(podir)/kdm.pot

EXTRA_DIST = Chooser.ad  chooser.cpp kdm-0.5.lsm kdm_on_RH5.0.html kdmusers.h \
	krb5auth.c sessreg.c sessreg.man xdm.man xdmconfig xdmshell.c
