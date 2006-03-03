kde4_header()

add_subdirectory( about ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kcontrol_KDEINIT_SRCS 
   main.cpp 
   toplevel.cpp 
   indexwidget.cpp 
   helpwidget.cpp 
   dockcontainer.cpp 
   aboutwidget.cpp 
   quickhelp.cpp 
   moduletreeview.cpp 
   moduleiconview.cpp 
   moduleIface.cpp 
   global.cpp 
   modules.cpp 
   proxywidget.cpp 
   kcrootonly.cpp 
   searchwidget.cpp )

kde4_automoc(${kcontrol_KDEINIT_SRCS})

kde4_add_dcop_skels(kcontrol_KDEINIT_SRCS moduleIface.h )

kde4_add_kdeinit_executable( kcontrol ${kcontrol_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcontrol  ${LIB_KDE3SUPPORT} khtml )

install_targets(${LIB_INSTALL_DIR} kdeinit_kcontrol )

target_link_libraries( kcontrol kdeinit_kcontrol )
install_targets(/bin kcontrol )

########### install files ###############

install_files( ${XDG_APPS_DIR} FILES KControl.desktop kinfocenter.desktop )
install_files( ${DATA_INSTALL_DIR}/kcontrol FILES kcontrolui.rc )
install_files( ${DATA_INSTALL_DIR}/kinfocenter FILES kinfocenterui.rc )

kde4_install_icons( ${ICON_INSTALL_DIR} crystalsvg )

kde4_footer()



#original Makefile.am contents follow:

##
##  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
##  Copyright (c) 2000 Matthias Elter <elter@kde.org>
## 
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
## 
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
## 
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
## 
#
#AM_CPPFLAGS= $(all_includes)
#METASOURCES = AUTO
#
#SUBDIRS = . about
#
#bin_PROGRAMS =
#lib_LTLIBRARIES =
#kdeinit_LTLIBRARIES = kcontrol.la
#
#kcontrol_la_SOURCES = main.cpp toplevel.cpp indexwidget.cpp \
#	helpwidget.cpp dockcontainer.cpp aboutwidget.cpp \
#	quickhelp.cpp moduletreeview.cpp moduleiconview.cpp \
#	moduleIface.cpp moduleIface.skel global.cpp modules.cpp \
#	proxywidget.cpp kcrootonly.cpp searchwidget.cpp
#
#kcontrol_la_LIBADD  = $(LIB_KDEUI) $(LIB_KUTILS) $(LIB_KHTML)
#kcontrol_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#rccontroldir = $(kde_datadir)/kcontrol
#rccontrol_DATA = kcontrolui.rc
#rcinfodir = $(kde_datadir)/kinfocenter
#rcinfo_DATA = kinfocenterui.rc
#
#xdg_apps_DATA = KControl.desktop kinfocenter.desktop
#
#install-data-local:
#	@-rm -f $(DESTDIR)$(kde_bindir)/kinfocenter
#	$(mkinstalldirs) $(DESTDIR)$(kde_bindir)
#	@$(LN_S) kcontrol $(DESTDIR)$(kde_bindir)/kinfocenter
#
#KDE_ICON=AUTO
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kcontrol.pot
