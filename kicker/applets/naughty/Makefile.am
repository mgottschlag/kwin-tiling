pic_DATA = naughty-happy.png naughty-sad.png
picdir = $(kde_datadir)/naughtyapplet/pics

INCLUDES = -I$(top_srcdir)/workspace/kicker/libkicker $(all_includes)

kde_module_LTLIBRARIES = naughty_panelapplet.la

naughty_panelapplet_la_SOURCES = \
  NaughtyProcessMonitor.cpp \
  NaughtyConfigDialog.cpp \
  NaughtyApplet.cpp

METASOURCES = AUTO

noinst_HEADERS = \
  NaughtyProcessMonitor.h \
  NaughtyConfigDialog.h \
  NaughtyApplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = naughtyapplet.desktop

EXTRA_DIST = $(lnk_DATA)

naughty_panelapplet_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
naughty_panelapplet_la_LIBADD = $(LIB_KDEUI) $(LIB_KVM) $(top_builddir)/workspace/kicker/libkicker/libkickermain.la 

KDE_ICON=AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp *.h -o $(podir)/naughtyapplet.pot

