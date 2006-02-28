kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### next target ###############

set(kfontviewpart_PART_SRCS FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp )

kde4_automoc(${kfontviewpart_PART_SRCS})

kde4_add_plugin(kfontviewpart WITH_PREFIX ${kfontviewpart_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kfontviewpart )

target_link_libraries(kfontviewpart  ${QT_AND_KDECORE_LIBS} kparts kfontinstprint kfontinst )

install_targets(${LIB_INSTALL_DIR}/kde4 kfontviewpart )


########### next target ###############

set(kfontview_SRCS FontViewerApp.cpp )

kde4_automoc(${kfontview_SRCS})

kde4_add_executable(kfontview ${kfontview_SRCS})

target_link_libraries(kfontview  ${QT_AND_KDECORE_LIBS} )

install_targets(/bin kfontview )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES kfontview.desktop )
install_files( ${DATA_INSTALL_DIR}/kfontview FILES kfontviewpart.rc kfontviewui.rc )
install_files( ${SERVICES_INSTALL_DIR} FILES kfontviewpart.desktop )

kde4_footer()



#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libkfontinstprint.la
#libkfontinstprint_la_SOURCES = KfiPrint.cpp
#libkfontinstprint_la_LDFLAGS = $(all_libraries)
#libkfontinstprint_la_LIBADD = $(LIB_KDEPRINT) ../lib/libkfontinst.la
#
#kde_module_LTLIBRARIES = libkfontviewpart.la
#
#libkfontviewpart_la_SOURCES = FontViewPart.cpp FontViewPartFactory.cpp FontPreview.cpp
#libkfontviewpart_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#libkfontviewpart_la_LIBADD = $(LIB_KPARTS) libkfontinstprint.la ../lib/libkfontinst.la
#
#noinst_HEADERS  = FontViewPart.h FontViewPartFactory.h FontPreview.h FontViewerApp.h KfiPrint.h
#
#kde_services_DATA = kfontviewpart.desktop
#
#AM_CPPFLAGS = -I$(srcdir)/../lib -I$(srcdir)/../../fonts $(all_includes) $(LIBFREETYPE_CFLAGS) $(LIBFONTCONFIG_CFLAGS)
#METASOURCES = AUTO
#
#kfontview_LDADD = $(LIB_KPARTS)
#kfontview_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#bin_PROGRAMS = kfontview
#xdg_apps_DATA = kfontview.desktop
#
#appdata_DATA = kfontviewpart.rc kfontviewui.rc
#appdatadir = $(kde_datadir)/kfontview
#
#kfontview_SOURCES = FontViewerApp.cpp
#
#
