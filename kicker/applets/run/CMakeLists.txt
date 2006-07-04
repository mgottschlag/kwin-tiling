


########### next target ###############

set(run_panelapplet_PART_SRCS runapplet.cpp )

kde4_automoc(${run_panelapplet_PART_SRCS})

kde4_add_plugin(run_panelapplet ${run_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} run_panelapplet )

target_link_libraries(run_panelapplet  ${KDE4_KDECORE_LIBS} kworkspace kdeui kickermain )

install(TARGETS run_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES runapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes) -I$(top_srcdir)/workspace/lib $(all_includes)
#
#kde_module_LTLIBRARIES = run_panelapplet.la
#
#run_panelapplet_la_SOURCES = runapplet.cpp
#
#METASOURCES = runapplet.moc
#noinst_HEADERS = runapplet.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = runapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#run_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#run_panelapplet_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la $(LIB_KSYCOCA) $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/krunapplet.pot
