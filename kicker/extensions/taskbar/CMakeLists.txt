include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/taskbar ${CMAKE_SOURCE_DIR}/workspace/kicker/taskmanager/   )


########### next target ###############

set(taskbar_panelextension_PART_SRCS taskbarextension.cpp )

kde4_automoc(${taskbar_panelextension_PART_SRCS})


kde4_add_plugin(taskbar_panelextension ${taskbar_panelextension_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} taskbar_panelextension )

target_link_libraries(taskbar_panelextension  ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} kickermain taskmanager taskbar )

install(TARGETS taskbar_panelextension  DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES taskbarextension.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/extensions )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )




#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../taskmanager -I$(srcdir)/../../taskbar -I$(srcdir)/../../libkicker -I$(top_builddir)/kicker/libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = taskbar_panelextension.la
#
#taskbar_panelextension_la_SOURCES = taskbarextension.cpp taskbarextension.skel
#taskbar_panelextension_la_METASOURCES = AUTO
#taskbar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#taskbar_panelextension_la_LIBADD = $(LIB_KDEUI) $(LIB_KPARTS) \
#                                   ../../libkicker/libkickermain.la \
#                                   ../../taskmanager/libtaskmanager.la \
#                                   ../../taskbar/libtaskbar.la
#KDE_ICON=AUTO
#
#noinst_HEADERS = taskbarextension.h
#
#lnkdir = $(kde_datadir)/kicker/extensions
#lnk_DATA = taskbarextension.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/taskbarextension.pot
#
#taskbarextension.lo: ../../libkicker/kickerSettings.h
