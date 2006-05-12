kde4_header()
include(MacroAddFileDependencies)
add_subdirectory( themer ) 
add_subdirectory( themes ) 
add_subdirectory( pics ) 
add_subdirectory( sessions ) 
include_directories( 
	${CMAKE_SOURCE_DIR}/workspace/kcontrol/background/ 
	${CMAKE_SOURCE_DIR}/workspace/kdm/backend  
	${CMAKE_SOURCE_DIR}/workspace/kdm/
	${CMAKE_SOURCE_DIR}/workspace/kdmlib/
	${CMAKE_BINARY_DIR}/workspace/kdm/
	 )


########### next target ###############
set(libkdmthemer_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmthemer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmitem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmpixmap.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmrect.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmlabel.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/themer/kdmlayout.cpp
	)



add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/workspace/kdm/config.ci COMMAND ${PERL_EXECUTABLE} ARGS -w ${CMAKE_SOURCE_DIR}/workspace/kdm/confproc.pl ${CMAKE_SOURCE_DIR}/workspace/kdm/config.def ${CMAKE_BINARY_DIR}/workspace/kdm/config.ci  DEPENDS ${CMAKE_SOURCE_DIR}/workspace/kdm/config.def )


macro_additional_clean_files( ${CMAKE_BINARY_DIR}/workspace/kdm/config.ci )

set(kdm_config_SRCS kdm_config.c ${kdmconfig_SRCS}  )


kde4_automoc(${kdm_config_SRCS})

kde4_add_executable(kdm_config ${kdm_config_SRCS})

target_link_libraries(kdm_config  ${KDE4_KDEUI_LIBS} )

install(TARGETS kdm_config DESTINATION bin)


########### next target ###############

set(kdm_greet_SRCS ${libkdmthemer_SRCS}
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

target_link_libraries(kdm_greet  ${KDE4_KDEUI_LIBS} ${X11_XTest_LIB})

install(TARGETS kdm_greet DESTINATION bin)


########### next target ###############

set(krootimage_SRCS krootimage.cpp ${backgroundlib_SRCS})

kde4_automoc(${krootimage_SRCS})

kde4_add_executable(krootimage ${krootimage_SRCS})

target_link_libraries(krootimage  ${KDE4_KIO_LIBS} )

install(TARGETS krootimage DESTINATION bin)


########### next target ###############

set(genkdmconf_SRCS genkdmconf.c )

#add_file_dependencies(${CMAKE_SOURCE_DIR}/workspace/kdm/config.ci genkdmconf.c )
macro_add_file_dependencies(genkdmconf.c ${CMAKE_BINARY_DIR}/workspace/kdm/config.ci )


kde4_automoc(${genkdmconf_SRCS})

kde4_add_executable(genkdmconf ${genkdmconf_SRCS})

target_link_libraries(genkdmconf  ${KDE4_KDECORE_LIBS} )

install(TARGETS genkdmconf DESTINATION bin)


########### next target ###############

set(kdmctl_SRCS kdmctl.c )

kde4_automoc(${kdmctl_SRCS})

kde4_add_executable(kdmctl ${kdmctl_SRCS})

target_link_libraries(kdmctl  ${KDE4_KDECORE_LIBS} )

install(TARGETS kdmctl DESTINATION bin)


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
