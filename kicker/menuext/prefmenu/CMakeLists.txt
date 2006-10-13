


########### next target ###############

set(kickermenu_prefmenu_PART_SRCS prefmenu.cpp )

kde4_automoc(kickermenu_prefmenu ${kickermenu_prefmenu_PART_SRCS})

kde4_add_plugin(kickermenu_prefmenu ${kickermenu_prefmenu_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kickermenu_prefmenu )

target_link_libraries(kickermenu_prefmenu  ${KDE4_KDE3SUPPORT_LIBS} kickermain kworkspace )

install(TARGETS kickermenu_prefmenu  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES prefmenu.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker -I$(top_builddir)/workspace/kicker/libkicker -I$(top_srcdir)/workspace/lib/ $(all_includes)
#
#kde_module_LTLIBRARIES = kickermenu_prefmenu.la
#
#kickermenu_prefmenu_la_SOURCES = prefmenu.cpp
#kickermenu_prefmenu_la_LDFLAGS = $(all_libraries) -module -avoid-version
#kickermenu_prefmenu_la_LIBADD = $(LIB_KDEUI) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la $(top_builddir)/workspace/lib/libkworkspace.la
#
#kickermenu_prefmenu_la_METASOURCES = AUTO
#
#desktopmenu_DATA = prefmenu.desktop
#desktopmenudir = $(kde_datadir)/kicker/menuext
#
#messages:
#	$(XGETTEXT) *.cpp -o $(podir)/libkickermenu_prefmenu.pot
#
#prefmenu.lo: ../../libkicker/kickerSettings.h
