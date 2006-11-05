


########### next target ###############

set(systemtray_panelapplet_PART_SRCS systemtrayapplet.cpp )

kde4_automoc(${systemtray_panelapplet_PART_SRCS})


kde4_add_plugin(systemtray_panelapplet ${systemtray_panelapplet_PART_SRCS})


target_link_libraries(systemtray_panelapplet  ${KDE4_KDECORE_LIBS} kickermain ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS systemtray_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES systemtrayapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )

kde4_install_icons( ${ICON_INSTALL_DIR}  )




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
