kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kickermenu_find_PART_SRCS findmenu.cpp )

kde4_automoc(${kickermenu_find_PART_SRCS})

kde4_add_plugin(kickermenu_find ${kickermenu_find_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_find )

target_link_libraries(kickermenu_find  ${KDE4_KDECORE_LIBS} kdeui kickermain kworkspace )

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_find )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES find.desktop )
install_files( ${DATA_INSTALL_DIR}/kicker/menuext/find FILES kfind.desktop websearch.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/
#
#kde_module_LTLIBRARIES = kickermenu_find.la
#
#kickermenu_find_la_SOURCES = findmenu.cpp
#kickermenu_find_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_find_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la
#
#kickermenu_find_la_METASOURCES = AUTO
#
#desktopmenu_DATA = find.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#findmenu_data_DATA = kfind.desktop websearch.desktop
#findmenu_datadir = $(kde_datadir)/kicker/menuext/find
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_find.pot
