/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

// BEWARE ! unbelievably messy code

#define _MENUEDIT_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "menuedit.h"

#include <kglobal.h>
#include <klocale.h>
#include <kaccel.h>
#include <kapp.h>
#include <dcopclient.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <kkeydialog.h>

#include <settings.h>
#include <action_data.h>

namespace KHotKeys
{

void khotkeys_init()
    {
    // I hope this works
    KGlobal::locale()->insertCatalogue("khotkeys");
    // CHECKME hack
    init_global_data( false, new QObject );
    }

static bool menuentry_match( QString s1, QString s2 )
    {
    if( s1.endsWith( ".desktop" ))
        s1.truncate( s1.length() - 8 );
    if( s1.endsWith( ".kdelnk" ))
        s1.truncate( s1.length() - 7 );
    int slash = s1.findRev( '/' );
    if( slash >= 0 )
        s1 = s1.mid( slash + 1 );
    s1 = s1.lower();
    if( s2.endsWith( ".desktop" ))
        s2.truncate( s2.length() - 8 );
    if( s2.endsWith( ".kdelnk" ))
        s2.truncate( s2.length() - 7 );
    slash = s2.findRev( '/' );
    if( slash >= 0 )
        s2 = s2.mid( slash + 1 );
    s2 = s2.lower();
    return s1 == s2;
    }
    
Menuentry_shortcut_action_data* khotkeys_get_menu_entry_internal2( 
    const Action_data_group* data_P, const QString& entry_P )
    {
    if( !data_P->enabled( false ))
        return NULL;
    for( Action_data_group::Iterator it = data_P->first_child();
         it;
         ++it )
        {
        if( !(*it)->enabled( true ))
            continue;
        if( Menuentry_shortcut_action_data* entry
            = dynamic_cast< Menuentry_shortcut_action_data* >( *it ))
            {
            if( entry->action() != NULL && menuentry_match( entry->action()->command_url(), entry_P ))
                    return entry;
            }
        if( Action_data_group* group = dynamic_cast< Action_data_group* >( *it ))
            {
            Menuentry_shortcut_action_data* data
                = khotkeys_get_menu_entry_internal2( group, entry_P );
            if( data != NULL )
                return data;
            }
        }
    return NULL;
    }
    
Action_data_group* khotkeys_get_menu_root( Action_data_group* data_P )
    {
    for( Action_data_group::Iterator it = data_P->first_child();
         it;
         ++it )
        if( Action_data_group* group = dynamic_cast< Action_data_group* >( *it ))
            {
            if( group->system_group() == Action_data_group::SYSTEM_MENUENTRIES )
                return group;
            }    
    return new Action_data_group( data_P, i18n( MENU_EDITOR_ENTRIES_GROUP_NAME ),
        i18n( "These entries were created using Menu Editor" ), new Condition_list( "", NULL ), // CHECKME tenhle condition list
        Action_data_group::SYSTEM_MENUENTRIES, true );
    }

Menuentry_shortcut_action_data* khotkeys_get_menu_entry_internal( Action_data_group* data_P,
    const QString& entry_P )
    {
    return khotkeys_get_menu_entry_internal2( khotkeys_get_menu_root( data_P ), entry_P );
    }

QString khotkeys_get_menu_shortcut( Menuentry_shortcut_action_data* data_P )
    {
    if( data_P->trigger() != NULL )
        return KKey( data_P->trigger()->keycode()).toString();
    return "";
    }
        
void khotkeys_send_reread_config()
    {
    QByteArray data;
    if( !kapp->dcopClient()->isAttached())
        kapp->dcopClient()->attach();
    if( !kapp->dcopClient()->isApplicationRegistered( "khotkeys" ))
        {
        kdDebug( 1217 ) << "launching new khotkeys daemon" << endl;
        KApplication::kdeinitExec( "khotkeys" );
        }
    else
        {
        QByteArray data;
        kapp->dcopClient()->send( "khotkeys*", "khotkeys", "reread_configuration()", data );
        kdDebug( 1217 ) << "telling khotkeys daemon to reread configuration" << endl;
        }
    }

QString khotkeys_get_menu_entry_shortcut( const QString& entry_P )
    {
    Settings settings;
    settings.read_settings( true );
    Menuentry_shortcut_action_data* entry
        = khotkeys_get_menu_entry_internal( settings.actions, entry_P );
    if( entry == NULL )
        {
        delete settings.actions;
        return "";
        }
    QString shortcut = khotkeys_get_menu_shortcut( entry );
    delete settings.actions;
    return shortcut;
    }    
    
bool khotkeys_menu_entry_moved( const QString& new_P, const QString& old_P )
    {
    Settings settings;
    settings.read_settings( true );
    Menuentry_shortcut_action_data* entry
        = khotkeys_get_menu_entry_internal( settings.actions, old_P );
    if( entry == NULL )
        {
        delete settings.actions;
        return false;
        }
    Action_data_group* parent = entry->parent();
    QString new_name = new_P;
    if( entry->name().startsWith( i18n( "K Menu - " )))
        new_name = i18n( "K Menu - " ) + new_P;
    Menuentry_shortcut_action_data* new_entry = new Menuentry_shortcut_action_data( parent,
        new_name, entry->comment(), entry->enabled( true ));
    new_entry->set_trigger( entry->trigger()->copy( new_entry ));
    new_entry->set_action( new Menuentry_action( new_entry, new_P ));
    delete entry;
    settings.write_settings();
    delete settings.actions;
    khotkeys_send_reread_config();
    return true;
    }

void khotkeys_menu_entry_deleted( const QString& entry_P )
    {
    Settings settings;
    settings.read_settings( true );
    Menuentry_shortcut_action_data* entry
        = khotkeys_get_menu_entry_internal( settings.actions, entry_P );
    if( entry == NULL )
        {
        delete settings.actions;
        return;
        }
    delete entry;
    settings.write_settings();
    delete settings.actions;
    khotkeys_send_reread_config();
    }

QString khotkeys_change_menu_entry_shortcut( const QString& entry_P,
    const QString& shortcut_P )
    {
    Settings settings;
    settings.read_settings( true );
    Menuentry_shortcut_action_data* entry
        = khotkeys_get_menu_entry_internal( settings.actions, entry_P );
    bool new_entry = ( entry == NULL );
    if( new_entry )
        {
        entry = new Menuentry_shortcut_action_data( NULL, i18n( "K Menu - " ) + entry_P,
            "" );
        entry->set_action( new Menuentry_action( entry, entry_P ));
        }
    else
        {
        // erase the trigger, i.e. replace with a copy with no trigger and no parent yet
        Menuentry_shortcut_action_data* entry_tmp = new Menuentry_shortcut_action_data( NULL,
            entry->name(), entry->comment(), entry->enabled( false ));
        entry_tmp->set_action( new Menuentry_action( entry_tmp, entry_P ));
        delete entry;
        entry = entry_tmp;
        }
    QString shortcut = "";
    // make sure the shortcut is valid
    shortcut = (KShortcut( shortcut_P )).toStringInternal();
    if( !shortcut.isEmpty())
        entry->set_trigger( new Shortcut_trigger( entry, KKey( shortcut ).keyCodeQt()));
    if( shortcut.isEmpty())
        {
        delete entry;            
        if( !new_entry ) // remove from config file
            {
            settings.write_settings();
            khotkeys_send_reread_config();
            }
        delete settings.actions;
        return "";
        }
    entry->reparent( khotkeys_get_menu_root( settings.actions ));
    settings.write_settings();
    khotkeys_send_reread_config();
    return shortcut;
    }

// CHECKME nejaka forma kontroly, ze tahle kl. kombinace neni pouzita pro jiny menuentry ?

} // namespace KHotKeys

//
// exported functions
//

void khotkeys_init()
    {
    KHotKeys::khotkeys_init();
    }
    
QString khotkeys_get_menu_entry_shortcut( const QString& entry_P )
    {
    return KHotKeys::khotkeys_get_menu_entry_shortcut( entry_P );
    }

bool khotkeys_menu_entry_moved( const QString& new_P, const QString& old_P )
    {
    return KHotKeys::khotkeys_menu_entry_moved( new_P, old_P );
    }

void khotkeys_menu_entry_deleted( const QString& entry_P )
    {
    KHotKeys::khotkeys_menu_entry_deleted( entry_P );
    }
    
QString khotkeys_change_menu_entry_shortcut( const QString& entry_P,
    const QString& shortcut_P )
    {
    return KHotKeys::khotkeys_change_menu_entry_shortcut( entry_P, shortcut_P );
    }
