# $Id$

SUBDIRS = greeter pics xdmconfig @kdmchooserdir@

# Extra lib. Might be -lpam -dl or -lshadow or nothing
PASSLIB = $(PASSWDLIB) $(PAMLIBS)

INCLUDES = $(all_includes) $(KERBEROS_INCS)

bin_PROGRAMS = kdm

EXTRA_PROGRAMS = greetest
greetest_SOURCES = greetest.c

AM_CFLAGS = $(STD_DEFINES) $(CONN_FLAGS)

# this is for static linking - inserted into KDM_GREET_LIBS by configure
KDM_KDE_LIBS  = $(LIB_KDEUI) -lXau $(PASSLIB) \
    $(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
    $(LIBCRYPT) $(LIBUCB) $(LIBUTIL) $(LIBRESOLV)

# Libraries:
LDFLAGS = $(all_libraries) $(KDE_RPATH) $(KRB_RPATH)
LDADD = $(LIB_X11) -lXau $(LIBXDMCP) $(PASSLIB) \
    $(LIB_LIBS) $(KERBEROS_LIBS) $(LIBSOCKET) \
    $(LIBCRYPT) $(LIBUCB) $(LIBDL) $(LIBUTIL) $(LIBRESOLV)
kdm_LDADD = $(LDADD) $(KDM_GREET_LIBS)

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
    krb5auth.c \
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

EXTRA_DIST = chooser misc
