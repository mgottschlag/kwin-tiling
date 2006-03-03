kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kicker/libkicker ${CMAKE_SOURCE_DIR}/workspace/lib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(lockout_panelapplet_PART_SRCS lockout.cpp )

kde4_automoc(${lockout_panelapplet_PART_SRCS})

kde4_add_plugin(lockout_panelapplet ${lockout_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} lockout_panelapplet )

target_link_libraries(lockout_panelapplet  ${QT_AND_KDECORE_LIBS} kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 lockout_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES lockout.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kicker/libkicker -I$(top_srcdir)/workspace/lib $(all_includes)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = lockout_panelapplet.la
#
#lockout_panelapplet_la_SOURCES = lockout.cpp
#lockout_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#lockout_panelapplet_la_LIBADD = $(LIB_KSYCOCA) $(top_builddir)/kicker/libkicker/libkickermain.la
#
#noinst_HEADERS = lockout.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = lockout.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/lockout.pot
