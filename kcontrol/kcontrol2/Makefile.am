bin_PROGRAMS = kcontrol2 kcmshell kcmroot

kcontrol2_SOURCES = main.cpp toplevel.cpp modules.cpp index.cpp utils.cpp \
			aboutwidget.cpp kdockwidget.cpp \
			proxywidget.cpp moduleinfo.cpp modloader.cpp \
			closedlg.cpp proxymodule.cpp KCModuleIface.stub

kcontrol2_LDADD   = $(LIB_KDEUI)


kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxymodule.cpp KCModuleIface.stub

kcmshell_LDADD   = $(LIB_KDEUI)

kcmroot_SOURCES = kcmroot.cpp KCModuleIface.skel moduleinfo.cpp modloader.cpp \
			utils.cpp KCModuleIface.stub \
			proxymodule.cpp

kcmroot_LDADD   = $(LIB_KDEUI)

icons_DATA = close.png
iconsdir = $(kde_datadir)/kcontrol2/pics

INCLUDES= $(all_includes)

METASOURCES = AUTO

LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages:
	$(XGETTEXT) -C -ki18n -x $(includedir)/kde.pot *.cpp -o ../../po/kcontrol2.pot

