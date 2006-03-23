kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/kicker/core  ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(trash_panelapplet_PART_SRCS trashbutton.cpp trashapplet.cpp )

kde4_automoc(${trash_panelapplet_PART_SRCS})

kde4_add_plugin(trash_panelapplet ${trash_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} trash_panelapplet )

target_link_libraries(trash_panelapplet  ${KDE4_KDECORE_LIBS} kickermain konq kdeui kio kutils )

install_targets(${LIB_INSTALL_DIR}/kde4 trash_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES trashapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/libkonq -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/kicker/core $(all_includes)
#
#kde_module_LTLIBRARIES = trash_panelapplet.la
#trash_panelapplet_la_SOURCES = trashbutton.cpp trashapplet.cpp 
#
#METASOURCES = AUTO
#
#noinst_HEADERS = \
#  trashapplet.h trashbutton.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = trashapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#trash_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#trash_panelapplet_la_LIBADD = ../../libkicker/libkickermain.la ../../../libkonq/libkonq.la $(LIB_KDEUI) $(LIB_KIO) $(LIB_KUTILS)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/trashapplet.pot
#
