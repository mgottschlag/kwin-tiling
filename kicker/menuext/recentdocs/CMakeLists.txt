kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kickermenu_recentdocs_PART_SRCS recentdocsmenu.cpp )

kde4_automoc(${kickermenu_recentdocs_PART_SRCS})

kde4_add_plugin(kickermenu_recentdocs ${kickermenu_recentdocs_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_recentdocs )

target_link_libraries(kickermenu_recentdocs  ${KDE4_KDECORE_LIBS} kdeui kickermain kworkspace kde3support)

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_recentdocs )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES recentdocs.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)
#
#kde_module_LTLIBRARIES = kickermenu_recentdocs.la
#
#kickermenu_recentdocs_la_SOURCES = recentdocsmenu.cpp
#kickermenu_recentdocs_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_recentdocs_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la
#
#kickermenu_recentdocs_la_METASOURCES = AUTO
#
#desktopmenu_DATA = recentdocs.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_recentdocs.pot
