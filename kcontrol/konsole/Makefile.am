# $Id$

kde_module_LTLIBRARIES = kcm_konsole.la

kcm_konsole_la_SOURCES =  schemaeditor.cpp sessioneditor.cpp kcmkonsole.cpp \
	schemadialog.ui sessiondialog.ui kcmkonsoledialog.ui

kcm_konsole_la_LDFLAGS  = $(all_libraries) -module -avoid-version -no-undefined
kcm_konsole_la_LIBADD = -lkdeui $(LIB_KFILE)
INCLUDES= $(all_includes) 

kcm_konsole_la_METASOURCES = AUTO

noinst_HEADERS = kcmkonsole.h schemaeditor.h sessioneditor.h schemadialog.h sessiondialog.h kcmkonsoledialog.h

KDE_ICON = konsole

data_DATA = kcmkonsole.desktop
datadir = $(kde_appsdir)/Settings/Personalization

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkonsole.pot
