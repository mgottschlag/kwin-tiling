########### next target ###############

set(kickermenu_kate_PART_SRCS katesessionmenu.cpp )

kde4_automoc(${kickermenu_kate_PART_SRCS})

kde4_add_plugin(kickermenu_kate ${kickermenu_kate_PART_SRCS})


target_link_libraries(kickermenu_kate kworkspace kickermain )

install(TARGETS kickermenu_kate DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES katesessionmenu.desktop DESTINATION  ${DATA_INSTALL_DIR}/kicker/menuext )


## Below is the content of the Makefile.am
# INCLUDES = $(all_includes)
#
# kde_module_LTLIBRARIES = kickermenu_kate.la
#
# kickermenu_kate_la_SOURCES = katesessionmenu.cpp
# kickermenu_kate_la_LDFLAGS = $(all_libraries) -module -avoid-version
# kickermenu_kate_la_LIBADD = $(LIB_KDEUI)
#
# kickermenu_kate_la_METASOURCES = AUTO
#
# desktopmenu_DATA = katesessionmenu.desktop
# desktopmenudir = $(kde_datadir)/kicker/menuext
#
# messages:
# 	$(XGETTEXT) *.cpp -o $(podir)/kickermenu_kate.pot