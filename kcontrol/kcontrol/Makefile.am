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
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
# 

INCLUDES= $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH) -export-dynamic
METASOURCES = AUTO

SUBDIRS = . pics

bin_PROGRAMS = kcontrol kcmshell kcminit

kcontrol_SOURCES = main.cpp toplevel.cpp indexwidget.cpp searchwidget.cpp \
	helpwidget.cpp moduleinfo.cpp utils.cpp modules.cpp proxywidget.cpp \
	modloader.cpp dockcontainer.cpp aboutwidget.cpp global.cpp \
	quickhelp.cpp moduletreeview.cpp moduleiconview.cpp
kcontrol_LDADD   = $(LIB_KDEUI)


kcmshell_SOURCES = kcmshell.cpp moduleinfo.cpp modloader.cpp \
			utils.cpp proxywidget.cpp kcdialog.cpp global.cpp
kcmshell_LDADD = $(LIB_KDEUI)


kcminit_SOURCES = kcminit.cpp
kcminit_LDADD   = $(LIB_KDEUI)

rcdir = $(kde_datadir)/kcontrol
rc_DATA = kcontrolui.rc

KDE_ICON = kcontrol

rc.cpp:
	$(EXTRACTRC) kcontrolui.rc >> rc.cpp

messages: rc.cpp
	$(XGETTEXT) rc.cpp $(kcontrol_SOURCES) -o $(podir)/kcontrol.pot
	$(XGETTEXT) $(kcmshell_SOURCES) -o $(podir)/kcmshell.pot
	$(XGETTEXT) $(kcminit_SOURCES) -o $(podir)/kcminit.pot
