bin_PROGRAMS = kcontrol2 kcmshell kcmroot

kcontrol2_SOURCES = main.cpp toplevel.cpp modules.cpp index.cpp utils.cpp \
			aboutwidget.cpp kdockwidget.cpp \
			proxywidget.cpp moduleinfo.cpp modloader.cpp \
			closedlg.cpp proxymodule.cpp KCModuleIface.stub

kcontrol2_LDADD   = -lkdeui -lkdecore -lqt -lXext -lX11


kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxymodule.cpp KCModuleIface.stub

kcmshell_LDADD   = -lkdeui -lkdecore -lqt -lXext -lX11

kcmroot_SOURCES = kcmroot.cpp KCModuleIface.skel moduleinfo.cpp modloader.cpp \
			utils.cpp KCModuleIface.stub \
			proxymodule.cpp

kcmroot_LDADD   = -lkdeui -lkdecore -lqt -lXext -lX11

icons_DATA = close.png
iconsdir = $(kde_datadir)/kcontrol2/pics

INCLUDES= $(all_includes)

METASOURCES = AUTO

LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

DISTCLEANFILES = $(kcontrol2_METASOURCES)

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot $(kcontrol2_SOURCES) -o ../po/kcontrol2.pot
