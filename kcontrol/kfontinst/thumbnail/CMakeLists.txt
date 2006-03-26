kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(fontthumbnail_PART_SRCS FontThumbnail.cpp )

kde4_automoc(${fontthumbnail_PART_SRCS})

kde4_add_plugin(fontthumbnail ${fontthumbnail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} fontthumbnail )

target_link_libraries(fontthumbnail  ${KDE4_KIO_LIBS} kfontinst )

install_targets(${LIB_INSTALL_DIR}/kde4 fontthumbnail )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES fontthumbnail.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES  )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = fontthumbnail.la
#
#fontthumbnail_la_SOURCES = FontThumbnail.cpp
#fontthumbnail_la_LIBADD = $(LIBFREETYPE_LIBS) $(LIB_KDECORE) $(LIB_KIO) ../lib/libkfontinst.la
#fontthumbnail_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
#METASOURCES = AUTO
#
#noinst_HEADERS = FontThumbnail.h
#
#kdelnkdir = $(kde_servicesdir)
#
#kde_services_DATA = fontthumbnail.desktop
#
#AM_CPPFLAGS= -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS)
