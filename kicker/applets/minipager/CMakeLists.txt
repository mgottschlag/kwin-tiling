include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/taskmanager ${CMAKE_SOURCE_DIR}/workspace/kicker/taskbar/  )


########### next target ###############

set(minipager_panelapplet_PART_SRCS pagerapplet.cpp pagerbutton.cpp)

kde4_automoc(${minipager_panelapplet_PART_SRCS})

kde4_add_kcfg_files(minipager_panelapplet_PART_SRCS pagersettings.kcfgc)

qt4_add_dbus_interface(minipager_panelapplet_PART_SRCS ../../../kdesktop/org.kde.kdesktop.Background.xml kdesktop_background_interface)

kde4_add_plugin(minipager_panelapplet ${minipager_panelapplet_PART_SRCS})

kde4_install_libtool_file(${PLUGIN_INSTALL_DIR} minipager_panelapplet)

target_link_libraries(minipager_panelapplet ${KDE4_KDEUI_LIBS} ${KDE4_KDEFX_LIBS} taskmanager kickermain)

install(TARGETS minipager_panelapplet  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES minipagerapplet.desktop  DESTINATION  ${DATA_INSTALL_DIR}/kicker/applets )




#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../taskmanager \
#	-I$(srcdir)/../../libkicker -I../../libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = minipager_panelapplet.la
#
#minipager_panelapplet_la_SOURCES = pagerapplet.cpp pagerbutton.cpp pagersettings.kcfgc
#
#minipager_panelapplet_la_METASOURCES = AUTO
#noinst_HEADERS = pagerapplet.h pagerbutton.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = minipagerapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA) 
#
#minipager_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#minipager_panelapplet_la_LIBADD = $(LIB_KDEUI) \
#	../../taskmanager/libtaskmanager.la \
#	../../libkicker/libkickermain.la
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/kminipagerapplet.pot
#
#srcdoc:
#	kdoc -a -p -H -d $$HOME/web/src/kminipagerapplet kminipagerapplet *.h -lqt -lkdecore -lkdeui -lkfile
