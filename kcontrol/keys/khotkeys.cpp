/*
 *   Copyright (C) 2000 Matthias Elter <elter@kde.org>
 *                      Lubos Lunak    <l.lunak@email.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <klibloader.h>
#include "khotkeys.h"

extern "C"
{
    static void (*khotkeys_init_2)( void );
    static void (*khotkeys_cleanup_2)( void );
    static QString (*khotkeys_get_menu_entry_shortcut_2)( const QString& entry_P );
    static QString (*khotkeys_change_menu_entry_shortcut_2)( const QString& entry_P,
                                                             const QString& shortcut_P );
    static bool (*khotkeys_menu_entry_moved_2)( const QString& new_P, const QString& old_P );
    static void (*khotkeys_menu_entry_deleted_2)( const QString& entry_P );
}

static bool khotkeys_present = false;
static bool khotkeys_inited = false;

bool KHotKeys::init()
{
    khotkeys_inited = true;
    KLibrary* lib = KLibLoader::self()->library( "kcm_khotkeys.la" );
    if( lib == NULL ) return false;

    khotkeys_init_2 = ( void (*)(void)) ( lib->resolveFunction( "khotkeys_init" ));
    khotkeys_cleanup_2 = ( void (*)(void)) ( lib->resolveFunction( "khotkeys_cleanup" ));
    khotkeys_get_menu_entry_shortcut_2 =
        ( QString (*)( const QString& ))
        ( lib->resolveFunction( "khotkeys_get_menu_entry_shortcut" ));
    khotkeys_change_menu_entry_shortcut_2 =
        ( QString (*)( const QString&, const QString& ))
        ( lib->resolveFunction( "khotkeys_change_menu_entry_shortcut" ));
    khotkeys_menu_entry_moved_2 =
        ( bool (*)( const QString&, const QString& ))
        ( lib->resolveFunction( "khotkeys_menu_entry_moved" ));
    khotkeys_menu_entry_deleted_2 =
        ( void (*)( const QString& ))
        ( lib->resolveFunction( "khotkeys_menu_entry_deleted" ));
    if( khotkeys_init_2
        && khotkeys_cleanup_2
        && khotkeys_get_menu_entry_shortcut_2
        && khotkeys_change_menu_entry_shortcut_2
        && khotkeys_menu_entry_moved_2
        && khotkeys_menu_entry_deleted_2 )
    {
        khotkeys_init_2();
        khotkeys_present = true;
        return true;
    }
    return false;
}

void KHotKeys::cleanup()
{
    if( khotkeys_inited )
        khotkeys_cleanup_2();
    khotkeys_inited = false;
}

bool KHotKeys::present()
{
    if( !khotkeys_inited )
        init();
    return khotkeys_present;
}

QString KHotKeys::getMenuEntryShortcut( const QString& entry_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return "";
    return khotkeys_get_menu_entry_shortcut_2( entry_P );
}

QString KHotKeys::changeMenuEntryShortcut( const QString& entry_P,
    const QString shortcut_P )
    {
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return "";
    return khotkeys_change_menu_entry_shortcut_2( entry_P, shortcut_P );
    }

bool KHotKeys::menuEntryMoved( const QString& new_P, const QString& old_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return "";
    return khotkeys_menu_entry_moved_2( new_P, old_P );
}

void KHotKeys::menuEntryDeleted( const QString& entry_P )
{
    if( !khotkeys_inited )
        init();
    if( !khotkeys_present )
        return;
    khotkeys_menu_entry_deleted_2( entry_P );
}
