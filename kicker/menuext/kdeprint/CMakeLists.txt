kde4_header()
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )


########### next target ###############

set(kickermenu_kdeprint_PART_SRCS print_mnu.cpp )

kde4_automoc(${kickermenu_kdeprint_PART_SRCS})

kde4_add_plugin(kickermenu_kdeprint ${kickermenu_kdeprint_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_kdeprint )

target_link_libraries(kickermenu_kdeprint  ${KDE4_KDECORE_LIBS} kdeui kio kdeprint kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 kickermenu_kdeprint )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/menuext FILES printmenu.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(top_srcdir)/workspace/kicker/libkicker
#
#kde_module_LTLIBRARIES = kickermenu_kdeprint.la
#
#kickermenu_kdeprint_la_SOURCES = print_mnu.cpp
#kickermenu_kdeprint_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_kdeprint_la_LIBADD = $(LIB_KDEUI) $(LIB_KIO) -lkdeprint $(top_builddir)/workspace/kicker/libkicker/libkickermain.la
#
#kickermenu_kdeprint_la_METASOURCES = AUTO
#
#desktopmenu_DATA = printmenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_kdeprint.pot
