include_directories( ${CMAKE_SOURCE_DIR}/workspace/plasma/lib/ ${CMAKE_SOURCE_DIR}/libkonq   )


########### next target ###############

set(sidebar_panelextension_PART_SRCS sidebarextension.cpp )

kde4_automoc(${sidebar_panelextension_PART_SRCS})

kde4_add_plugin(sidebar_panelextension ${sidebar_panelextension_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} sidebar_panelextension )

target_link_libraries(sidebar_panelextension  ${KDE4_KDECORE_LIBS} kickermain kdeui kparts konq )

install(TARGETS sidebar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES sidebarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )




#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../libkicker -I$(top_srcdir)/libkonq $(all_includes)
#
#kde_module_LTLIBRARIES = sidebar_panelextension.la
#
#sidebar_panelextension_la_SOURCES = sidebarextension.cpp
#
#sidebar_panelextension_la_METASOURCES = AUTO
#sidebar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#sidebar_panelextension_la_LIBADD =  ../../libkicker/libkickermain.la $(LIB_KDEUI) $(LIB_KPARTS) ../../../libkonq/libkonq.la
#
#
#lnkdir = $(kde_datadir)/kicker/extensions
#lnk_DATA = sidebarextension.desktop
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kickersidebarextension.pot
