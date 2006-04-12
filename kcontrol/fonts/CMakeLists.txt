kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### next target ###############


########### next target ###############

set(kcm_fonts_PART_SRCS fonts.cpp kxftconfig.cpp ${libkrd_SRCS} )

kde4_automoc(${kcm_fonts_PART_SRCS})

kde4_add_plugin(kcm_fonts ${kcm_fonts_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_fonts )

target_link_libraries(kcm_fonts  ${KDE4_KDEUI_LIBS} ${FONTCONFIG_LIBRARIES})

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_fonts )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES fonts.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libkxftconfig.la
#
#libkxftconfig_la_SOURCES = kxftconfig.cpp
#libkxftconfig_la_LIBADD = $(LIBFONTCONFIG_LIBS)
#libkxftconfig_la_LDFLAGS = $(LIBFONTCONFIG_RPATH)
#
#AM_CPPFLAGS  = $(all_includes) $(LIBFONTCONFIG_CFLAGS) -D_LARGEFILE64_SOURCE
#
#kde_module_LTLIBRARIES = kcm_fonts.la
#
#kcm_fonts_la_SOURCES = fonts.cpp
#kcm_fonts_la_LDFLAGS = $(KDE_RPATH) $(LIBFONTCONFIG_RPATH) -module -avoid-version $(all_libraries) -no-undefined
#kcm_fonts_la_LIBADD = libkxftconfig.la $(top_builddir)/workspace/kcontrol/krdb/libkrdb.la $(LIB_KDEUI) $(LIBFONTCONFIG_LIBS)
#METASOURCES = AUTO
#
#noinst_HEADERS = fonts.h kxftconfig.h
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcmfonts.pot
#
#xdg_apps_DATA = fonts.desktop
#
#KDE_ICON=AUTO
