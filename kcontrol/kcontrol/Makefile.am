SUBDIRS = . pics

bin_PROGRAMS = kcontrol kcmshell kcmroot kcminit

kcontrol_SOURCES = main.cpp toplevel.cpp modules.cpp index.cpp utils.cpp \
			aboutwidget.cpp kdockwidget.cpp \
			proxywidget.cpp moduleinfo.cpp modloader.cpp \
			closedlg.cpp proxymodule.cpp KCModuleIface.stub

kcontrol_LDADD   = $(LIB_KDEUI)


kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxymodule.cpp KCModuleIface.stub


kcmshell_LDADD   = $(LIB_KDEUI)

kcmroot_SOURCES = kcmroot.cpp KCModuleIface.skel moduleinfo.cpp modloader.cpp \
			utils.cpp KCModuleIface.stub \
			proxymodule.cpp

kcmroot_LDADD   = $(LIB_KDEUI)


kcminit_SOURCES = kcminit.cpp

kcminit_LDADD   = $(LIB_KDEUI)


icons_DATA = close.png
iconsdir = $(kde_datadir)/kcontrol/pics

INCLUDES= $(all_includes)

METASOURCES = AUTO

KDE_ICON = kcontrol

LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages:
	$(XGETTEXT) -C -kI18N_NOOP -ki18n -x $(includedir)/kde.pot *.cpp -o $(podir)/kcontrol.pot

