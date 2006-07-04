


########### next target ###############

set(kickermenu_recentdocs_PART_SRCS recentdocsmenu.cpp )

kde4_automoc(${kickermenu_recentdocs_PART_SRCS})

kde4_add_plugin(kickermenu_recentdocs ${kickermenu_recentdocs_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_recentdocs )

target_link_libraries(kickermenu_recentdocs  ${KDE4_KDEUI_LIBS} kickermain kworkspace kde3support)

install(TARGETS kickermenu_recentdocs  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES recentdocs.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )




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
