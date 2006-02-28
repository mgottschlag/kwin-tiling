kde4_header()

add_subdirectory( themer ) 
add_subdirectory( themes ) 
add_subdirectory( pics ) 
add_subdirectory( sessions ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kdm_config_SRCS kdm_config.c )

kde4_automoc(${kdm_config_SRCS})

kde4_add_executable(kdm_config ${kdm_config_SRCS})

target_link_libraries(kdm_config  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdm_config )


########### next target ###############

set(kdm_greet_SRCS 
   kdm_greet.c 
   kdmconfig.cpp 
   kdmclock.cpp 
   kconsole.cpp 
   kfdialog.cpp 
   kgdialog.cpp 
   kchooser.cpp 
   kgverify.cpp 
   kdmshutdown.cpp 
   kgreeter.cpp 
   kgapp.cpp )

kde4_automoc(${kdm_greet_SRCS})

kde4_add_executable(kdm_greet ${kdm_greet_SRCS})

target_link_libraries(kdm_greet  ${QT_AND_KDECORE_LIBS} kdeui )

install_targets(/bin kdm_greet )


########### next target ###############

set(krootimage_SRCS krootimage.cpp )

kde4_automoc(${krootimage_SRCS})

kde4_add_executable(krootimage ${krootimage_SRCS})

target_link_libraries(krootimage  ${QT_AND_KDECORE_LIBS} bgnd kio )

install_targets(/bin krootimage )


########### next target ###############

set(genkdmconf_SRCS genkdmconf.c )

kde4_automoc(${genkdmconf_SRCS})

kde4_add_executable(genkdmconf ${genkdmconf_SRCS})

target_link_libraries(genkdmconf  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin genkdmconf )


########### next target ###############

set(kdmctl_SRCS kdmctl.c )

kde4_automoc(${kdmctl_SRCS})

kde4_add_executable(kdmctl ${kdmctl_SRCS})

target_link_libraries(kdmctl  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kdmctl )


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

## use 'make GENKDMCONF_FLAGS=... install' to override
#GENKDMCONF_FLAGS =
#
#SUBDIRS = themer themes pics sessions
#
#AM_CPPFLAGS = -I$(srcdir)/../backend -I.. -I$(top_srcdir)/workspace/kcontrol/background \
#    -I$(top_srcdir)/kdmlib $(all_includes)
#
#bin_PROGRAMS = kdm_config kdm_greet krootimage genkdmconf kdmctl
#
#kdm_config_SOURCES = kdm_config.c
#kdm_config_LDADD = $(LIBRESOLV) $(LIBSOCKET) $(LIBPOSIX4)
#
#kdm_greet_SOURCES = \
#    kdm_greet.c \
#    kdmconfig.cpp \
#    kdmclock.cpp \
#    kconsole.cpp \
#    kfdialog.cpp \
#    kgdialog.cpp \
#    kchooser.cpp \
#    kgverify.cpp \
#    kdmshutdown.cpp \
#    kgreeter.cpp \
#    kgapp.cpp
#kdm_greet_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#kdm_greet_LDADD = themer/libkdmthemer.a $(LIB_KDEUI) $(XTESTLIB) $(LIBPOSIX4)
#
#krootimage_SOURCES = krootimage.cpp
#krootimage_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#krootimage_LDADD = $(top_builddir)/workspace/kcontrol/background/libbgnd.la $(LIB_KIO)
#
#METASOURCES = AUTO
#
#genkdmconf_SOURCES = genkdmconf.c
#genkdmconf_LDFLAGS = $(X_LDFLAGS) $(X_RPATH)
#genkdmconf_LDADD = $(LIB_X11)
#
#kdmctl_SOURCES = kdmctl.c
#kdmctl_LDADD = $(LIBSOCKET)
#
#install-data-local: genkdmconf
#	./genkdmconf --in $(DESTDIR)$(kde_confdir)/kdm --face-src $(srcdir)/pics $(GENKDMCONF_FLAGS)
#
#messages:
#	$(XGETTEXT) `find . -name "*.cpp"` -o $(podir)/kdmgreet.pot
#
#noinst_HEADERS = \
#    kdm_greet.h \
#    kdmconfig.h \
#    kdmclock.h \
#    kconsole.h \
#    kfdialog.h \
#    kgdialog.h \
#    kchooser.h \
#    kgverify.h \
#    kdmshutdown.h \
#    kgreeter.h \
#    kgapp.h \
#    \
#    krootimage.h 
#
#kdm_greet_COMPILE_FIRST = ../config.ci
#kdm_config_COMPILE_FIRST = ../config.ci
#genkdmconf_COMPILE_FIRST = ../config.ci
#
