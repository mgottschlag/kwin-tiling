kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  ${CMAKE_SOURCE_DIR}/workspace/kicker/taskbar/ ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${CMAKE_SOURCE_DIR}/workspace/kicker/taskmanager )


########### next target ###############

set(taskbar_panelapplet_PART_SRCS taskbarapplet.cpp )

kde4_automoc(${taskbar_panelapplet_PART_SRCS})

kde4_add_plugin(taskbar_panelapplet ${taskbar_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} taskbar_panelapplet )

target_link_libraries(taskbar_panelapplet  ${QT_AND_KDECORE_LIBS} kdeui taskbar kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 taskbar_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES taskbarapplet.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../taskbar -I$(srcdir)/../../taskmanager -I$(srcdir)/../../libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = taskbar_panelapplet.la
#
#taskbar_panelapplet_la_SOURCES = taskbarapplet.cpp
#
#taskbar_panelapplet_la_METASOURCES = AUTO
#taskbar_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#taskbar_panelapplet_la_LIBADD = $(LIB_KDEUI) ../../taskbar/libtaskbar.la ../../libkicker/libkickermain.la
#
#noinst_HEADERS = taskbarapplet.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = taskbarapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#messages:
#	$(XGETTEXT) *.cpp *.h -o $(podir)/ktaskbarapplet.pot
#
#srcdoc:
#	kdoc -a -p -H -d $$HOME/web/src/ktaskbarapplet ktaskbarapplet *.h -lqt -lkdecore -lkdeui -lkfile
