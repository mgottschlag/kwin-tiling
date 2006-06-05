
project(kcminit)



########### next target ###############

set(kcminit_KDEINIT_SRCS main.cpp )


kde4_automoc(${kcminit_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcminit ${kcminit_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit  ${KDE4_KIO_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR} kdeinit_kcminit )

target_link_libraries( kcminit kdeinit_kcminit )
install(TARGETS kcminit DESTINATION bin)

########### next target ###############

# TODO might be simpler to make <whatever>_startup to be a symlink to <whatever>

set(kcminit_startup_KDEINIT_SRCS main.cpp )


kde4_automoc(${kcminit_startup_KDEINIT_SRCS})

kde4_add_kdeinit_executable( kcminit_startup ${kcminit_startup_KDEINIT_SRCS})

target_link_libraries(kdeinit_kcminit_startup  ${KDE4_KIO_LIBS} kutils )

install_targets(${LIB_INSTALL_DIR} kdeinit_kcminit_startup )

target_link_libraries( kcminit_startup kdeinit_kcminit_startup )
install(TARGETS kcminit_startup DESTINATION bin)

########### install files ###############





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
#kdeinit_LTLIBRARIES = kcminit.la
#bin_PROGRAMS=
#lib_LTLIBRARIES=
#
#kcminit_la_SOURCES = main.cpp
#kcminit_la_LIBADD = $(LIB_KDEUI) $(LIB_KUTILS)
#kcminit_la_LDFLAGS = $(all_libraries) -module -avoid-version
#
#AM_CPPFLAGS = $(all_includes)
#METASOURCES = AUTO
#
