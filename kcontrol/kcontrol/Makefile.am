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

AM_CPPFLAGS= $(all_includes)
METASOURCES = AUTO

SUBDIRS = . about

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kcontrol.la

kcontrol_la_SOURCES = main.cpp toplevel.cpp indexwidget.cpp \
	helpwidget.cpp dockcontainer.cpp aboutwidget.cpp \
	quickhelp.cpp moduletreeview.cpp moduleiconview.cpp \
	moduleIface.cpp moduleIface.skel global.cpp modules.cpp \
	proxywidget.cpp kcrootonly.cpp searchwidget.cpp

kcontrol_la_LIBADD  = $(LIB_KDEUI) $(LIB_KUTILS) $(LIB_KHTML)
kcontrol_la_LDFLAGS = $(all_libraries) -module -avoid-version

rccontroldir = $(kde_datadir)/kcontrol
rccontrol_DATA = kcontrolui.rc
rcinfodir = $(kde_datadir)/kinfocenter
rcinfo_DATA = kinfocenterui.rc

xdg_apps_DATA = KControl.desktop

install-data-local: KControl_NoDisplay.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)
	$(INSTALL_DATA) $(srcdir)/KControl_NoDisplay.desktop $(DESTDIR)$(kde_appsdir)/KControl.desktop
	@-rm -f $(DESTDIR)$(kde_bindir)/kinfocenter
	$(mkinstalldirs) $(DESTDIR)$(kde_bindir)
	@$(LN_S) kcontrol $(DESTDIR)$(kde_bindir)/kinfocenter
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/System
	$(INSTALL_DATA) $(srcdir)/kinfocenter.desktop $(DESTDIR)$(kde_appsdir)/System/

KDE_ICON=AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot
