


########### next target ###############

set(kickermenu_kdeprint_PART_SRCS print_mnu.cpp )

kde4_automoc(${kickermenu_kdeprint_PART_SRCS})

kde4_add_plugin(kickermenu_kdeprint ${kickermenu_kdeprint_PART_SRCS})


target_link_libraries(kickermenu_kdeprint  ${KDE4_KIO_LIBS} ${KDE4_KDEPRINT_LIBS} kickermain )

install(TARGETS kickermenu_kdeprint  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES printmenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )




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
