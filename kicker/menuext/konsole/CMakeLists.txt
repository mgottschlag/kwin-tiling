kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kickermenu_konsole_PART_SRCS konsole_mnu.cpp konsolebookmarkhandler.cpp konsolebookmarkmenu.cpp )

kde4_automoc(${kickermenu_konsole_PART_SRCS})

kde4_add_plugin(kickermenu_konsole ${kickermenu_konsole_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_konsole )

target_link_libraries(kickermenu_konsole  ${KDE4_KDE3SUPPORT_LIBS} kworkspace kickermain  ${QT_QTXML_LIBRARY} )

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_konsole )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES konsolemenu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)
#
#kde_module_LTLIBRARIES = kickermenu_konsole.la
#
#kickermenu_konsole_la_SOURCES = konsole_mnu.cpp konsolebookmarkhandler.cpp konsolebookmarkmenu.cpp
#kickermenu_konsole_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_konsole_la_LIBADD = $(LIB_KIO) $(top_builddir)/workspace/lib/libkworkspace.la
#
#kickermenu_konsole_la_METASOURCES = AUTO
#
#desktopmenu_DATA = konsolemenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_konsole.pot
