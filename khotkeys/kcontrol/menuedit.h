/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#ifndef _MENUEDIT_H_
#define _MENUEDIT_H_

#include <qstring.h>
#include <kdialogbase.h>
#include <kaccel.h>

// see also kdebase/kmenuedit/khotkeys.h
extern "C"
    {
// initializes khotkeys DSO - loads i18n catalogue
// handled automatically by KHotKeys wrapper class in kmenuedit
void khotkeys_init( void );
// return keyboard shortcut ( e.g. "ALT+T" ) for given menu entry ( e.g.
// "System/Konsole.desktop"
QString khotkeys_get_menu_entry_shortcut( const QString& entry_P );
// changes assigned shortcut to menu entry a updates config file
QString khotkeys_change_menu_entry_shortcut( const QString& entry_P,
    const QString& shortcut_P );
// menu entry was moved in K Menu
bool khotkeys_menu_entry_moved( const QString& new_P, const QString& old_P );
// menu entry was removed
void khotkeys_menu_entry_deleted( const QString& entry_P );    
    } // extern "C"

#endif
