kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kickermenu_remotemenu_PART_SRCS remotemenu.cpp )

kde4_automoc(${kickermenu_remotemenu_PART_SRCS})

kde4_add_dcop_skels(kickermenu_remotemenu_PART_SRCS remotemenu.h )

kde4_add_plugin(kickermenu_remotemenu ${kickermenu_remotemenu_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_remotemenu )

target_link_libraries(kickermenu_remotemenu  ${QT_AND_KDECORE_LIBS} kdeui kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_remotemenu )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES remotemenu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_builddir)/workspace/kicker/libkicker
#
#kde_module_LTLIBRARIES = kickermenu_remotemenu.la
#
#kickermenu_remotemenu_la_SOURCES = remotemenu.cpp remotemenu.skel
#kickermenu_remotemenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_remotemenu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la
#
#kickermenu_remotemenu_la_METASOURCES = AUTO
#
#desktopmenu_DATA = remotemenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_remotemenu.pot
#
#remotemenu.lo: ../../libkicker/kickerSettings.h
