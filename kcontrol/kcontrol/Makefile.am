#
#  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
#  Copyright (c) 2000 Matthias Elter <elter@kde.org>
# 
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
# 
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
# 

INCLUDES= $(all_includes)
METASOURCES = AUTO

SUBDIRS = . pics

CLEANFILES = dummy.cpp

bin_PROGRAMS = kcontrol kcmshell kcminit
lib_LTLIBRARIES = kcontrol.la kcmshell.la kcminit.la

kcontrol_la_SOURCES = main.cpp toplevel.cpp indexwidget.cpp searchwidget.cpp \
	helpwidget.cpp modules.cpp proxywidget.cpp \
	dockcontainer.cpp aboutwidget.cpp global.cpp \
	quickhelp.cpp moduletreeview.cpp moduleiconview.cpp \
	kcrootonly.cpp moduleIface.cpp moduleIface.skel

kcontrol_la_LIBADD  = $(LIB_KDEUI) $(LIB_KSYCOCA) -lkutils 
kcontrol_la_LDFLAGS = $(all_libraries) -module -avoid-version
kcmshell_la_SOURCES = kcmshell.cpp proxywidget.cpp \
	                  kcdialog.cpp global.cpp kcdialog.skel
kcmshell_la_LIBADD = $(LIB_KDEUI) $(LIB_KSYCOCA) -lkutils 
kcmshell_la_LDFLAGS = $(all_libraries) -module -avoid-version


kcminit_la_SOURCES = kcminit.cpp global.cpp
kcminit_la_LIBADD   = $(LIB_KDEUI) $(LIB_KSYCOCA) -lkutils
kcminit_la_LDFLAGS = $(all_libraries) -module -avoid-version

kcontrol_SOURCES = dummy.cpp
kcontrol_LDADD = kcontrol.la
kcontrol_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

kcmshell_SOURCES = dummy.cpp
kcmshell_LDADD = kcmshell.la
kcmshell_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

kcminit_SOURCES = dummy.cpp
kcminit_LDADD = kcminit.la
kcminit_LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic

rccontroldir = $(kde_datadir)/kcontrol
rccontrol_DATA = kcontrolui.rc
rcinfodir = $(kde_datadir)/kinfocenter
rcinfo_DATA = kinfocenterui.rc

kcontrol_data_DATA = KControl.desktop
kcontrol_datadir = $(kde_appsdir)/Settingsmenu

install-data-local: KControl_NoDisplay.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)
	$(INSTALL_DATA) $(srcdir)/KControl_NoDisplay.desktop $(DESTDIR)$(kde_appsdir)/KControl.desktop
	@-rm -f $(DESTDIR)$(kde_bindir)/kinfocenter
	$(mkinstalldirs) $(DESTDIR)$(kde_bindir)
	@$(LN_S) kcontrol $(DESTDIR)$(kde_bindir)/kinfocenter
	$(INSTALL_DATA) $(srcdir)/kinfocenter.desktop $(DESTDIR)$(kde_appsdir)/System/

KDE_ICON=AUTO

EXTRA_DIST = $(kcontrol_data_DATA)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot

dummy.cpp:
	echo > dummy.cpp
