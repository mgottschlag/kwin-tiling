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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# 

kdeinit_LTLIBRARIES = kcminit.la kcminit_startup.la
bin_PROGRAMS=
lib_LTLIBRARIES=

kcminit_la_SOURCES = main.cpp main.skel
kcminit_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)
kcminit_la_LDFLAGS = $(all_libraries) -module -avoid-version

kcminit_startup_la_SOURCES = main.cpp main.skel
kcminit_startup_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)
kcminit_startup_la_LDFLAGS = $(all_libraries) -module -avoid-version

AM_CPPFLAGS = $(all_includes)
METASOURCES = AUTO
