kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/libkonq ${CMAKE_SOURCE_DIR}/kdmlib ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = -I$(srcdir)/../core -I../core -I$(srcdir)/../buttons \
#	-I../../libkicker -I$(srcdir)/../../libkicker \
#	-I$(top_srcdir)/libkonq -I$(top_srcdir)/kdmlib \
#	-I$(top_srcdir)/workspace/lib $(all_includes)
#
#noinst_LTLIBRARIES = libkicker_ui.la
#
#libkicker_ui_la_SOURCES = addbutton_mnu.cpp appletitem.ui appletview.ui addapplet.cpp \
#        addapplet_mnu.cpp appletop_mnu.cpp \
#        browser_mnu.cpp client_mnu.cpp \
#        nonKDEButtonSettings.ui exe_dlg.cpp k_mnu.cpp k_mnu.skel\
#        quickbrowser_mnu.cpp service_mnu.cpp \
#        addextension_mnu.cpp extensionop_mnu.cpp \
#        recentapps.cpp browser_dlg.cpp \
#        removeapplet_mnu.cpp removeextension_mnu.cpp removecontainer_mnu.cpp \
#        removebutton_mnu.cpp hidebutton.cpp addappletvisualfeedback.cpp
#
#libkicker_ui_la_LIBADD = $(top_builddir)/workspace/lib/libkworkspace.la $(top_builddir)/libkonq/libkonq.la $(top_builddir)/kdmlib/libdmctl.la
#
#libkicker_ui_la_METASOURCES = AUTO
#
#noinst_HEADERS = addapplet.h appletwidget.h addbutton_mnu.h addapplet_mnu.h appletop_mnu.h \
#		browser_mnu.h client_mnu.h exe_dlg.h k_mnu.h \
#		quickbrowser_mnu.h service_mnu.h \
#		addextension_mnu.h extensionop_mnu.h \
#		recentapps.h browser_dlg.h \
#		removeapplet_mnu.h removeextension_mnu.h removecontainer_mnu.h \
#		removebutton_mnu.h hidebutton.h addappletvisualfeedback.h
#
#removecontainer_mnu.lo: ../../libkicker/kickerSettings.h
#removeextension_mnu.lo: ../../libkicker/kickerSettings.h
#addextension_mnu.lo: ../../libkicker/extensionSettings.h
#appletop_mnu.lo: ../../libkicker/kickerSettings.h
#extensionop_mnu.lo: ../../libkicker/kickerSettings.h
#k_mnu.lo: ../../libkicker/kickerSettings.h
#removecontainer_mnu.lo: ../../libkicker/extensionSettings.h
#removeextension_mnu.lo: ../../libkicker/extensionSettings.h
#service_mnu.lo: ../../libkicker/kickerSettings.h
#browser_mnu.lo: ../../libkicker/kickerSettings.h
#recentapps.lo: ../../libkicker/kickerSettings.h
#
