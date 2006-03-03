kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(menu_panelapplet_PART_SRCS menuapplet.cpp )

kde4_automoc(${menu_panelapplet_PART_SRCS})

kde4_add_dcop_skels(menu_panelapplet_PART_SRCS menuapplet.h )

kde4_add_plugin(menu_panelapplet ${menu_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} menu_panelapplet )

target_link_libraries(menu_panelapplet  ${QT_AND_KDECORE_LIBS} kdeui kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 menu_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES menuapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kicker/libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = menu_panelapplet.la
#
#menu_panelapplet_la_SOURCES = menuapplet.cpp menuapplet.skel
#
#noinst_HEADERS = menuapplet.h
#
#menu_panelapplet_la_METASOURCES = AUTO
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = menuapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#menu_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#menu_panelapplet_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/kicker/libkicker/libkickermain.la
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kmenuapplet.pot
