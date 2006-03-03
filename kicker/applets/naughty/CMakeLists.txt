kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/workspace/kicker/libkicker ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(naughty_panelapplet_PART_SRCS NaughtyProcessMonitor.cpp NaughtyConfigDialog.cpp NaughtyApplet.cpp )

kde4_automoc(${naughty_panelapplet_PART_SRCS})

kde4_add_plugin(naughty_panelapplet ${naughty_panelapplet_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} naughty_panelapplet )

target_link_libraries(naughty_panelapplet  ${QT_AND_KDECORE_LIBS} kdeui kickermain )

install_targets(${LIB_INSTALL_DIR}/kde4 naughty_panelapplet )


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker/applets FILES naughtyapplet.desktop )
install_files( ${DATA_INSTALL_DIR}/naughtyapplet/pics FILES naughty-happy.png naughty-sad.png )

kde4_install_icons( hicolor crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

#pic_DATA = naughty-happy.png naughty-sad.png
#picdir = $(kde_datadir)/naughtyapplet/pics
#
#INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)
#
#kde_module_LTLIBRARIES = naughty_panelapplet.la
#
#naughty_panelapplet_la_SOURCES = \
#  NaughtyProcessMonitor.cpp \
#  NaughtyConfigDialog.cpp \
#  NaughtyApplet.cpp
#
#METASOURCES = AUTO
#
#noinst_HEADERS = \
#  NaughtyProcessMonitor.h \
#  NaughtyConfigDialog.h \
#  NaughtyApplet.h
#
#lnkdir = $(kde_datadir)/kicker/applets
#lnk_DATA = naughtyapplet.desktop
#
#EXTRA_DIST = $(lnk_DATA)
#
#naughty_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
#naughty_panelapplet_la_LIBADD = $(LIB_KDEUI) $(LIB_KVM) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la 
#
#KDE_ICON=AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp *.h -o $(podir)/naughtyapplet.pot
#
