SUBDIRS = . pics

bin_PROGRAMS = kcontrol kcmshell kcminit

kcontrol_SOURCES = main.cpp toplevel.cpp indexwidget.cpp searchwidget.cpp \
	helpwidget.cpp moduleinfo.cpp utils.cpp modules.cpp proxywidget.cpp \
	modloader.cpp dockcontainer.cpp aboutwidget.cpp kcdialog.cpp global.cpp \
	quickhelp.cpp

kcontrol_LDADD   = $(LIB_KDEUI)

kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxywidget.cpp kcdialog.cpp global.cpp


kcmshell_LDADD = $(LIB_KDEUI)

kcminit_SOURCES = kcminit.cpp

kcminit_LDADD   = $(LIB_KDEUI)

INCLUDES= $(all_includes)

rcdir = $(kde_datadir)/kcontrol
rc_DATA = kcontrolui.rc

METASOURCES = AUTO

KDE_ICON = kcontrol

LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot

