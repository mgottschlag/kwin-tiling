SUBDIRS = pics

bin_PROGRAMS = kcontrol kcmshell kcmroot

kcontrol_SOURCES = main.cpp toplevel.cpp modules.cpp index.cpp utils.cpp \
			aboutwidget.cpp kdockwidget.cpp \
			proxywidget.cpp moduleinfo.cpp modloader.cpp \
			closedlg.cpp proxymodule.cpp KCModuleIface.stub

kcontrol_LDADD   = $(LIB_KDEUI) $(LIB_KDECORE)


kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxymodule.cpp KCModuleIface.stub

kcmshell_LDADD   = $(LIB_KDEUI)

kcmroot_SOURCES = kcmroot.cpp KCModuleIface.skel moduleinfo.cpp modloader.cpp \
			utils.cpp KCModuleIface.stub \
			proxymodule.cpp

modloader.cpp: proxymodule.h
toplevel.cpp: proxymodule.h
proxymodule.h: KCModuleIface_stub.h

kcmroot_LDADD   = $(LIB_KDEUI)

icons_DATA = close.png
iconsdir = $(kde_datadir)/kcontrol/pics

INCLUDES= $(all_includes)

METASOURCES = AUTO

KDE_ICON = kcontrol

LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot *.cpp -o ../../po/kcontrol.pot

