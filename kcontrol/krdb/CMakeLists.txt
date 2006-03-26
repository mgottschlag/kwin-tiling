kde4_header()

add_subdirectory( themes ) 
add_subdirectory( kcs ) 
add_subdirectory( ad ) 
add_subdirectory( pics ) 
include_directories( ${KDE4_INCLUDES} )


########### next target ###############


########### install files ###############

install_files( ${KCONF_UPDATE_INSTALL_DIR} FILES kcmdisplayrc.upd )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes)
#
#SUBDIRS = . themes kcs ad pics
#
#noinst_LTLIBRARIES = libkrdb.la
#libkrdb_la_SOURCES = krdb.cpp
#libkrdb_la_LIBADD = $(LIB_KDECORE)
#libkrdb_la_LDFLAGS = -no-undefined $(all_libraries) 
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/krdb.pot
#
#update_DATA = kcmdisplayrc.upd
#updatedir = $(kde_datadir)/kconf_update
