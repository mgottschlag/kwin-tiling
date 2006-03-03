kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kicker/libkicker ${CMAKE_SOURCE_DIR}/workspace/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(run_panelapplet_PART_SRCS runapplet.cpp )

kde4_automoc(${run_panelapplet_PART_SRCS})

kde4_add_plugin(run_panelapplet ${run_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} run_panelapplet )

target_link_libraries(run_panelapplet  ${QT_AND_KDECORE_LIBS} kworkspace kdeui kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 run_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES runapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kicker/libkicker $(all_includes) -I$(top_srcdir)/workspace/lib $(all_includes)
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
#run_panelapplet_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la $(LIB_KSYCOCA) $(LIB_KDEUI) $(top_builddir)/kicker/libkicker/libkickermain.la
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/krunapplet.pot
