# $Id$

SUBDIRS	= $(KDM_CHOOSER_DIR) greeter config pics $(xdmconfigsubdir)

# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB= $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes) $(KERBEROS_INCS)

bin_PROGRAMS = kdm

noinst_PROGRAMS = greetest
greetest_SOURCES = greetest.c

# Libraries:
LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KRB_RPATH)
#
LDADD  = $(LIB_X11) -lXau $(LIBXDMCP) $(PASSLIB) \
	$(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
	$(LIBCRYPT) $(LIBUCB) $(LIBDL) $(LIBUTIL) 

# Sources for kdm:
kdm_SOURCES = access.c \
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
	xdmauth.c \
	xdmcp.c

noinst_HEADERS = kdm-config.h dm.h dm_socket.h dm_error.h dm_auth.h greet.h

EXTRA_DIST = misc xdmconfig
