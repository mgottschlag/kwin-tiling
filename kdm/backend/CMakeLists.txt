include_directories(${CMAKE_SOURCE_DIR}/workspace/kdm/  )


########### next target ###############

set(kdm_SRCS ${kdmconfig_SRCS} 
   access.c 
   auth.c 
   bootman.c 
   choose.c 
   client.c 
   ctrl.c 
   daemon.c 
   dm.c 
   dpylist.c 
   error.c 
   genauth.c 
   inifile.c 
   krb5auth.c 
   mitauth.c 
   netaddr.c 
   policy.c 
   process.c 
   protodpy.c 
   reset.c 
   resource.c 
   rpcauth.c 
   server.c 
   session.c 
   sessreg.c 
   socket.c 
   streams.c 
   util.c 
   xdmauth.c 
   xdmcp.c )

kde4_automoc(${kdm_SRCS})

kde4_add_executable(kdm ${kdm_SRCS})

target_link_libraries(kdm  ${KDE4_KDECORE_LIBS} ${X11_Xdmcp_LIB} ${X11_Xau_LIB} ${PAM_LIBRARIES})

install(TARGETS kdm DESTINATION bin)


########### install files ###############





#original Makefile.am contents follow:

## forcibly remove thread-related defines & flags
#AUTOMAKE_OPTIONS = foreign
#CPPFLAGS = $(USER_INCLUDES) $(X_INCLUDES) $(KRB4_INCS) $(KRB5_INCS) -I.. -I../..
#LDFLAGS = $(USER_LDFLAGS) $(X_LDFLAGS) $(X_RPATH) $(KRB4_RPATH) $(KRB5_RPATH)
#LDADD = $(LIB_X11) -lXau $(LIBXDMCP) $(PASSWDLIBS) \
#    $(LIB_LIBS) $(KRB4_LIBS) $(KRB5_LIBS) $(LIBSOCKET) $(LIBRESOLV) \
#    $(LIBUCB) $(LIBUTIL) $(LIBPOSIX4)
#
#bin_PROGRAMS = kdm
#kdm_SOURCES = \
#    access.c \
#    auth.c \
#    bootman.c \
#    choose.c \
#    client.c \
#    ctrl.c \
#    daemon.c \
#    dm.c \
#    dpylist.c \
#    error.c \
#    genauth.c \
#    inifile.c \
#    krb5auth.c \
#    mitauth.c \
#    netaddr.c \
#    policy.c \
#    process.c \
#    protodpy.c \
#    reset.c \
#    resource.c \
#    rpcauth.c \
#    server.c \
#    session.c \
#    sessreg.c \
#    socket.c \
#    streams.c \
#    util.c \
#    xdmauth.c \
#    xdmcp.c
#
#EXTRA_DIST = printf.c
#
#noinst_HEADERS = dm.h dm_socket.h dm_error.h dm_auth.h greet.h
#
## for unsermake (automake is handled by SUBDIRS in ../)
#kdm_COMPILE_FIRST = ../config.ci
