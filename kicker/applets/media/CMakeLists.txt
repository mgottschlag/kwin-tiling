kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/kicker/core   )


########### next target ###############

set(media_panelapplet_PART_SRCS preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp )

kde4_automoc(${media_panelapplet_PART_SRCS})

kde4_add_plugin(media_panelapplet ${media_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} media_panelapplet )

target_link_libraries(media_panelapplet  ${KDE4_KDECORE_LIBS} konq kickermain kdeui kio kutils )

install_targets(${PLUGIN_INSTALL_DIR} media_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES mediaapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libkonq -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/kicker/core $(all_includes)
#
#kde_module_LTLIBRARIES = media_panelapplet.la
#media_panelapplet_la_SOURCES = preferencesdialog.cpp mediumbutton.cpp mediaapplet.cpp
#
#METASOURCES = AUTO
#
#noinst_HEADERS = mediaapplet.h mediumbutton.h preferencesdialog.h 
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = mediaapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#media_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#media_panelapplet_la_LIBADD = ../../../libkonq/libkonq.la ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_KUTILS)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/mediaapplet.pot
#
