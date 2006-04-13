kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/workspace/kcontrol/kfontinst/lib/ ${KDE4_INCLUDES} )


########### next target ###############

set(kfile_font_PART_SRCS KFileFont.cpp )

kde4_automoc(${kfile_font_PART_SRCS})

kde4_add_plugin(kfile_font ${kfile_font_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfile_font )

target_link_libraries(kfile_font  ${KDE4_KIO_LIBS} kfontinst )

install_targets(${LIB_INSTALL_DIR}/kde4 kfile_font )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kfile_font.desktop )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kfile_font.la
#
#kfile_font_la_SOURCES = KFileFont.cpp
#kfile_font_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIBFREETYPE_RPATH) -module $(KDE_PLUGIN)
#kfile_font_la_LIBADD = $(LIB_KIO) $(LIBFREETYPE_LIBS) ../lib/libkfontinst.la
#
#kdelnkdir = $(kde_servicesdir)
#kde_services_DATA = kfile_font.desktop
#
#AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS)
#
#METASOURCES = AUTO
#noinst_HEADERS = KFileFont.h
