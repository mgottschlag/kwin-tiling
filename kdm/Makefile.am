# $Id$

SUBDIRS	= config pics $(xdmconfigsubdir)

# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes)

#AM_CFLAGS   = -Wall -Wno-implicit -Wno-return-type

bin_PROGRAMS = kdm chooser

# Libraries:
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD  = $(LIB_KIMGIO) $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) $(LIB_LIBS) \
	$(LIBSOCKET) $(LIBCRYPT) $(LIBUCB) $(LIBUTIL) 

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
	kdmview.cpp \
	kfdialog.cpp \
	kgreeter.cpp \
	liloinfo.cpp \
	$(kdmlib_source)

chooser_SOURCES = \
        chooser.cpp \
	CXdmcp.cpp \
	DXdmcp.cpp \
	kdmview.cpp \
	kfdialog.cpp

chooser_LDFLAGS = $(all_libraries) $(KDE_RPATH)
chooser_LDADD  = $(LIB_KIMGIO) $(LIB_KDEUI) -lXau $(LIBXDMCP) $(PASSLIB) $(LIB_LIBS) \
	$(LIBSOCKET) $(LIBCRYPT) $(LIBUCB) $(LIBUTIL)  -lXt

noinst_HEADERS = kdmconfig.h kdmview.h kdm-config.h kdmshutdown.h kfdialog.h dm.h kgreeter.h greet.h liloinfo.h

METASOURCES = kfdialog.moc kdmview.moc kdmshutdown.moc kgreeter.moc liloinfo.moc CXdmcp.moc DXdmcp.moc

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kdm_SOURCES) $(kdmdesktop_SOURCES) && mv messages.po $(podir)/kdm.pot

EXTRA_DIST = Chooser.ad  chooser.cpp kdm-0.5.lsm kdm_on_RH5.0.html kdmusers.h \
	krb5auth.c sessreg.c sessreg.man xdm.man xdmconfig xdmshell.c

