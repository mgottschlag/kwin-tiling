#!/bin/sh -f

#////////////////////////////////////////////////////////////////////////////////
#//
#// Class Name    : kcmfontinst_install_fonts.sh
#// Author        : Craig Drummond
#// Project       : K Font Installer (kfontinst-kcontrol)
#// Creation Date : 03/08/2002
#// Version       : $Revision$ $Date$
#//
#////////////////////////////////////////////////////////////////////////////////
#//
#// This program is free software; you can redistribute it and/or
#// modify it under the terms of the GNU General Public License
#// as published by the Free Software Foundation; either version 2
#// of the License, or (at your option) any later version.
#//
#// This program is distributed in the hope that it will be useful,
#// but WITHOUT ANY WARRANTY; without even the implied warranty of
#// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#// GNU General Public License for more details.
#//
#// You should have received a copy of the GNU General Public License
#// along with this program; if not, write to the Free Software
#// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#//
#////////////////////////////////////////////////////////////////////////////////
#// (C) Craig Drummond, 2002
#////////////////////////////////////////////////////////////////////////////////

#
# Simple shell script to lauch kcmfontinst KControl module to install fonts...

if test $# -eq 0 ; then
    exit
fi

#
# First check if kcmfontinst is loaded into KControl...
kfi_app="kcontrol font_installer"
dcopfind $kfi_app
if test $? -eq 0; then
    #
    # ...yup, found in KControl
    status="kcontrol"
else
    #
    # ...ok, not loaded into KControl, is one loaded into a kcmshell???
    kfi_app="kcmshell_kcmfontinst font_installer"
    dcopfind $kfi_app
    if test $? -eq 0; then
        #
        # ...yup, found in kcmshell
        status="kcmshell"
    else
        #
        # ...hmmm, need to start a kcmshell then...
        kcmshell kcmfontinst &
        kdialog --msgbox "The Font Installer is currently being started\nplease wait..." &
        max_count=30
        count=0
        dcopfind $kfi_app
        while test $? -ne 0 && test $count -ne $max_count; do
            sleep 1
            count=`expr $count + 1`
            dcopfind $kfi_app
        done

        kill -9 $!
        if test $count -ne $max_count; then
            status="kcmshell"
        fi
    fi
fi

#
# Did we find a kcmfontinst, or create one?
if test -n "$status"; then
    #
    # Yipee, can now install the fonts...

    # First wait a little to see if the module is ready...
    max_count=360
    count=0
    res=`dcop $kfi_app ready`
    status=$?
    while test "$status" -eq "0" && test "$res" != "true" && test $count -ne $max_count; do
        sleep 1
        count=`expr $count + 1`
        res=`dcop $kfi_app ready`
        status=$?
    done

    if test $status -eq 0 && test "$res" == "true" && test $count -ne $max_count; then
        list=
        for font in $* ; do
            list="$font:$list"
        done
        if test -n "$list"; then
            dcop $kfi_app installFonts $list
        fi
    else
        kdialog --error "Timeout whilst waiting for Font Installer to initialise!"
    fi
else
    #
    # Something went wrong...
    kdialog --error "The Font Installer could not be started"
fi
