kde4_header()
include_directories( ${CMAKE_SOURCE_DIR}/libkonq  ${KDE4_INCLUDES} )


########### next target ###############


########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kicker FILES default-apps )
install_files( ${DATA_INSTALL_DIR}/kicker/extensions FILES childpanelextension.desktop )

kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../../libkicker -I../../libkicker \
#	 -I$(srcdir)/../ui -I$(srcdir)/../buttons -I$(top_srcdir)/libkonq \
#	 $(all_includes)
#
#noinst_LTLIBRARIES = libkicker_core.la
#
#libkicker_core_la_SOURCES = main.cpp kicker.cpp kicker.skel \
#    userrectsel.cpp containerarea.cpp \
#    applethandle.cpp container_base.cpp container_button.cpp \
#    container_applet.cpp container_extension.cpp extensionmanager.cpp \
#    menumanager.cpp pluginmanager.cpp showdesktop.cpp \
#    containerarealayout.cpp unhidetrigger.cpp \
#    panelextension.cpp panelextension.skel
#
#
#libkicker_core_la_LDFLAGS = $(all_libraries)
#libkicker_core_la_LIBADD = $(top_builddir)/libkonq/libkonq.la $(LIB_KDEUI)
#libkicker_core_la_METASOURCES = AUTO
#libkicker_core_la_COMPILE_FIRST = ../../libkicker/kickerSettings.h
#
#kicker_core_data_DATA = default-apps
#kicker_core_datadir = $(kde_datadir)/kicker
#
#lnkdir = $(kde_datadir)/kicker/extensions
#lnk_DATA = childpanelextension.desktop
#
#kicker.lo: ../../libkicker/kickerSettings.h
#showdesktop.lo: ../../libkicker/kickerSettings.h
#container_applet.lo: ../../libkicker/kickerSettings.h
#containerarea.lo: ../../libkicker/kickerSettings.h
#container_extension.lo: ../../libkicker/kickerSettings.h
#
