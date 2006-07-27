
include_directories(
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/
    ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/viewpart/
     )


########### next target ###############

set(fontthumbnail_PART_SRCS FontThumbnail.cpp )

kde4_automoc(${fontthumbnail_PART_SRCS})

kde4_add_plugin(fontthumbnail ${fontthumbnail_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} fontthumbnail )

target_link_libraries(fontthumbnail  ${KDE4_KIO_LIBS} kfontinst )

install(TARGETS fontthumbnail  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES fontthumbnail.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )




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
