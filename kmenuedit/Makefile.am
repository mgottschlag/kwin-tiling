INCLUDES = $(all_includes)

bin_PROGRAMS = kmenuedit
lib_LTLIBRARIES = kmenuedit.la

CLEANFILES = dummy.cpp

kmenuedit_la_SOURCES = namedlg.cpp advancedtab.cpp basictab.cpp treeview.cpp desktopfileeditor.cpp \
			menueditview.cpp kmenuedit.cpp main.cpp khotkeys.cpp
kmenuedit_la_LIBADD = $(LIB_KDEUI) $(LIB_KFILE)
kmenuedit_la_LDFLAGS = $(all_libraries) -module -avid-version

kmenuedit_SOURCES = dummy.cpp
kmenuedit_LDADD   = kmenuedit.la
kmenuedit_LDFLAGS = $(all_libraries) $(KDE_RPATH)

noinst_HEADERS = kmenuedit.h menueditview.h treeview.h desktopfileeditor.h basictab.h \
		advancedtab.h namedlg.h khotkeys.h

METASOURCES = kmenuedit.moc menueditview.moc treeview.moc desktopfileeditor.moc \
		basictab.moc advancedtab.moc

apps_DATA = kmenuedit.desktop
appsdir = $(kde_appsdir)/System

EXTRA_DIST = $(apps_DATA)

rcdir = $(kde_datadir)/kmenuedit
rc_DATA = kmenueditui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp -o $(podir)/kmenuedit.pot

dummy.cpp:
	echo > dummy.cpp
