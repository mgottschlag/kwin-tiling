kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(systemtray_panelapplet_PART_SRCS systemtrayapplet.cpp )

kde4_automoc(${systemtray_panelapplet_PART_SRCS})

kde4_add_dcop_skels(systemtray_panelapplet_PART_SRCS systemtrayapplet.h )

kde4_add_plugin(systemtray_panelapplet ${systemtray_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} systemtray_panelapplet )

target_link_libraries(systemtray_panelapplet  ${QT_AND_KDECORE_LIBS} kickermain kdeui kio )

install_targets(${LIB_INSTALL_DIR}/kde4 systemtray_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES systemtrayapplet.desktop )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#
#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = systemtray_panelapplet.la
#
#systemtray_panelapplet_la_SOURCES = systemtrayapplet.cpp systemtrayapplet.skel
#
#systemtray_panelapplet_la_METASOURCES = AUTO
#noinst_HEADERS = systemtrayapplet.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = systemtrayapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#systemtray_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#systemtray_panelapplet_la_LIBADD = ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO)
#
#KDE_ICON=AUTO
#
#messages:
#	$(XGETTEXT)  *.cpp *.h -o $(podir)/ksystemtrayapplet.pot
#
#srcdoc:
#	kdoc -a -p -H -d $$HOME/web/src/ksystemtrayapplet ksystemtrayapplet *.h -lqt -lkdecore -lkdeui -lkfile
