kde4_header()
include_directories( ${KDE4_INCLUDES} )


########### next target ###############

set(dockbar_panelextension_PART_SRCS dockcontainer.cpp dockbarextension.cpp )

kde4_automoc(${dockbar_panelextension_PART_SRCS})

kde4_add_plugin(dockbar_panelextension ${dockbar_panelextension_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} dockbar_panelextension )

target_link_libraries(dockbar_panelextension  ${KDE4_KDECORE_LIBS} kickermain kdeui )

install_targets(${PLUGIN_INSTALL_DIR} dockbar_panelextension )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/extensions FILES dockbarextension.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = dockbar_panelextension.la
#
#dockbar_panelextension_la_SOURCES = dockcontainer.cpp dockbarextension.cpp
#dockbar_panelextension_la_METASOURCES = AUTO
#dockbar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#dockbar_panelextension_la_LIBADD = ../../libkicker/libkickermain.la $(LIB_KDEUI)
#
#noinst_HEADERS = dockcontainer.h dockbarextension.h
#
#lnkdir = $(kde_datadir)/kicker/extensions
#lnk_DATA = dockbarextension.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/dockbarextension.pot
