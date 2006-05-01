INCLUDES = -I$(srcdir)/../../taskmanager -I$(srcdir)/../../taskbar -I$(srcdir)/../../libkicker -I../../libkicker $(all_includes)

kde_module_LTLIBRARIES = taskbar_panelextension.la

taskbar_panelextension_la_SOURCES = taskbarextension.cpp taskbarextension.skel
taskbar_panelextension_la_METASOURCES = AUTO
taskbar_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
taskbar_panelextension_la_LIBADD = $(LIB_KDEUI) $(LIB_KPARTS) \
                                   ../../libkicker/libkickermain.la \
                                   ../../taskmanager/libtaskmanager.la \
                                   ../../taskbar/libtaskbar.la
KDE_ICON=AUTO

noinst_HEADERS = taskbarextension.h

lnkdir = $(kde_datadir)/kicker/extensions
lnk_DATA = taskbarextension.desktop

EXTRA_DIST = $(lnk_DATA)


taskbarextension.lo: ../../libkicker/kickerSettings.h
