kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_fonts_PART_SRCS KioFonts.cpp )

kde4_automoc(${kio_fonts_PART_SRCS})

kde4_add_plugin(kio_fonts ${kio_fonts_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_fonts )

target_link_libraries(kio_fonts  ${KDE4_KDECORE_LIBS} kxftconfig kio kdesu kfontinst )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_fonts )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/konqsidebartng/virtual_folders/services FILES fonts.desktop )
install_files( ${MIME_INSTALL_DIR}/fonts FILES folder.desktop system-folder.desktop package.desktop )
install_files( ${SERVICES_INSTALL_DIR} FILES fonts.protocol )

kde4_footer()



#original Makefile.am contents follow:

#kde_module_LTLIBRARIES = kio_fonts.la
#kio_fonts_la_SOURCES = \
#KioFonts.cpp
#
#kio_fonts_la_LIBADD = ../../fonts/libkxftconfig.la $(LIBFONTCONFIG_LIBS) $(LIBFREETYPE_LIBS) $(LIB_KIO) -lkdesu ../lib/libkfontinst.la
#kio_fonts_la_LDFLAGS = $(KDE_PLUGIN) $(all_libraries) $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) $(LIBFREETYPE_RPATH) -module -avoid-version -no-undefined
#AM_CPPFLAGS= -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS) -D_LARGEFILE64_SOURCE
#
## The kxftconfig stuf really belongs to kdebase/kcontrol/fonts - here only so that can distribute this as an archive.
#noinst_HEADERS = \
#KioFonts.h
#
#servicesdir = $(kde_servicesdir)
#services_DATA = fonts.protocol
#
#vfsdata_DATA = fonts.desktop
#vfsdatadir = $(kde_datadir)/konqsidebartng/virtual_folders/services
#
#mimetype_DATA = folder.desktop system-folder.desktop package.desktop
#mimetypedir = $(kde_mimedir)/fonts
