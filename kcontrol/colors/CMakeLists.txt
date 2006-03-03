kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcm_colors_PART_SRCS colorscm.cpp widgetcanvas.cpp ${libkrd_SRCS})

kde4_automoc(${kcm_colors_PART_SRCS})

kde4_add_plugin(kcm_colors ${kcm_colors_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_colors )

target_link_libraries(kcm_colors  ${LIB_KDE3SUPPORT} kdefx )

install_targets(${LIB_INSTALL_DIR}/kde4 kcm_colors )


########### install files ###############

install_files( ${XDG_APPS_DIR} FILES colors.desktop )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = $(all_includes)
#
#kde_module_LTLIBRARIES = kcm_colors.la
#
#kcm_colors_la_SOURCES = colorscm.cpp widgetcanvas.cpp 
#kcm_colors_la_LDFLAGS = $(KDE_RPATH) -module -avoid-version $(all_libraries) -no-undefined
#kcm_colors_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) $(top_builddir)/workspace/kcontrol/krdb/libkrdb.la $(LIB_KDE3SUPPORT) 
#METASOURCES = colorscm.moc widgetcanvas.moc
#
#noinst_HEADERS = colorscm.h widgetcanvas.h
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/kcmcolors.pot
#
#xdg_apps_DATA = colors.desktop
#
#KDE_ICON=AUTO
#
