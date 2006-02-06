/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _TAB_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tab_widget.h"

#include <typeinfo>

#include <klocale.h>
#include <kdebug.h>

#include <actions.h>
#include <action_data.h>

#include "general_tab.h"
#include "action_group_tab.h"
#include "windowdef_list_widget.h"
#include "menuentry_widget.h"
#include "command_url_widget.h"
#include "dcop_widget.h"
#include "triggers_tab.h"
#include "info_tab.h"
#include "action_list_widget.h"
#include "keyboard_input_widget.h"
#include "kcmkhotkeys.h"
#include "condition_list_widget.h"
#include "gesture_triggers_tab.h"
#include "gestures_settings_tab.h"
#include "general_settings_tab.h"

// CHECKME
//nejak lip ty typeid
//asi spis udelat funkci, ktera vrati enum a pak tam mit switche, tak se zajisti, ze se nikde
//nezapomene

// CHECKME mozna nejak lepe to poradi tabu, takhle je to podle jejich poradi v enum

namespace KHotKeys
{

Tab_widget::Tab_widget( QWidget* parent_P, const char* name_P )
    : QTabWidget( parent_P, name_P )
    {
    pages[ TAB_INFO ] = new Info_tab;
    pages[ TAB_GENERAL_SETTINGS ] = new General_settings_tab;
    pages[ TAB_GESTURES_SETTINGS ] = new Gestures_settings_tab;
    General_tab* general_tab;
    pages[ TAB_GENERAL ] = general_tab = new General_tab;
    connect( general_tab, SIGNAL( action_type_changed( int )),
        SLOT( set_action_type_slot( int )));
    pages[ TAB_GROUP_GENERAL ] = new Action_group_tab;
    pages[ TAB_CONDITIONS ] = new Condition_list_tab;
    pages[ TAB_ACTIONS ] = new Action_list_tab;
    pages[ TAB_TRIGGERS ] = new Triggers_tab;
    pages[ TAB_SHORTCUT_TRIGGER ] = new Shortcut_trigger_tab;
    pages[ TAB_GESTURE_TRIGGER ] = new Gesture_triggers_tab;
    pages[ TAB_COMMAND_URL ] = new Command_url_tab;
    pages[ TAB_MENUENTRY ] = new Menuentry_tab;
    pages[ TAB_DCOP ] = new Dcop_tab;
    pages[ TAB_KEYBOARD_INPUT ] = new Keyboard_input_tab;
    pages[ TAB_WINDOW ] = new Windowdef_list_tab;
    for( tab_pos_t i = TAB_FIRST;
         i < TAB_END;
         ++i )
        connect( this, SIGNAL( clear_pages_signal()), pages[ i ], SLOT( clear_data()));
    show_pages(( TAB_INFO, TAB_GENERAL_SETTINGS, TAB_GESTURES_SETTINGS ));
    current_type = NONE;
    current_data_type = TYPE_GENERIC;
    }
    
Tab_widget::~Tab_widget()
    {
    for( tab_pos_t i = TAB_FIRST;
         i < TAB_END;
         ++i )
        {
        removePage( pages[ i ] );
        delete pages[ i ];
        }
    }
    
void Tab_widget::save_current_action_changes()
    {
    if( current_type == NONE ) // info, global settings
        {
        static_cast< Gestures_settings_tab* >( pages[ TAB_GESTURES_SETTINGS ] )->write_data(); // saves
        static_cast< General_settings_tab* >( pages[ TAB_GENERAL_SETTINGS ] )->write_data(); // saves
        }
    else if( current_type == GROUP )
        {
        Action_data_group* old =
            static_cast< Action_data_group* >( module->current_action_data());
        Action_data_group* item =
            static_cast< Action_group_tab* >( pages[ TAB_GROUP_GENERAL ] )->get_data(
            module->current_action_data()->parent(), NULL );
        item->set_conditions( static_cast< Condition_list_tab* >( pages[ TAB_CONDITIONS ] )
            ->get_data( item ));
        for( Action_data_group::Iterator it = old->first_child();
             it;
            )
            {
            Action_data_base* tmp = ( *it );
            ++it; // the items will be removed from the list, so better be carefull
            tmp->reparent( item );
            }
        module->set_current_action_data( item );
        }
    else if( current_type == DATA )
        {
        QString name, comment;
        bool enabled;
        static_cast< General_tab* >( pages[ TAB_GENERAL ] )->get_data( name, comment, enabled );
        switch( current_data_type )
            {
            case TYPE_GENERIC:
                {
                Generic_action_data* tmp = new Generic_action_data(
                    module->current_action_data()->parent(), name, comment, NULL, NULL, NULL,
                    enabled );
                tmp->set_triggers(
                    static_cast< Triggers_tab* >( pages[ TAB_TRIGGERS ] )->get_data( tmp ));
                tmp->set_conditions( static_cast< Condition_list_tab* >
                    ( pages[ TAB_CONDITIONS ] )->get_data( tmp ));
                tmp->set_actions(
                    static_cast< Action_list_tab* >( pages[ TAB_ACTIONS ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_COMMAND_URL_SHORTCUT:
                {
                Command_url_shortcut_action_data* tmp = new Command_url_shortcut_action_data( 
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_trigger(
                    static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action(
                    static_cast< Command_url_tab* >( pages[ TAB_COMMAND_URL ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_MENUENTRY_SHORTCUT:
                {
                Menuentry_shortcut_action_data* tmp = new Menuentry_shortcut_action_data( 
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_trigger(
                    static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action(
                    static_cast< Menuentry_tab* >( pages[ TAB_MENUENTRY ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_DCOP_SHORTCUT:
                {
                Dcop_shortcut_action_data* tmp = new Dcop_shortcut_action_data(
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_trigger(
                    static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action(
                    static_cast< Dcop_tab* >( pages[ TAB_DCOP ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_KEYBOARD_INPUT_SHORTCUT:
                {
                Keyboard_input_shortcut_action_data* tmp
                    = new Keyboard_input_shortcut_action_data(
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_trigger(
                    static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action( static_cast< Keyboard_input_tab* >
                    ( pages[ TAB_KEYBOARD_INPUT ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_KEYBOARD_INPUT_GESTURE:
                {
                Keyboard_input_gesture_action_data* tmp
                    = new Keyboard_input_gesture_action_data(
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_triggers(
                    static_cast< Gesture_triggers_tab* >( pages[ TAB_GESTURE_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action( static_cast< Keyboard_input_tab* >
                    ( pages[ TAB_KEYBOARD_INPUT ] )->get_data( tmp ));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_ACTIVATE_WINDOW_SHORTCUT:
                {
                Activate_window_shortcut_action_data* tmp
                    = new Activate_window_shortcut_action_data(
                    module->current_action_data()->parent(), name, comment, enabled );
                tmp->set_trigger(
                    static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->get_data( tmp ));
                tmp->set_action( new Activate_window_action( tmp,
                    static_cast< Windowdef_list_tab* >
                    ( pages[ TAB_WINDOW ] )->get_data()));
                module->set_current_action_data( tmp );
              break;
                }
            case TYPE_END:
              assert( false );
            }
        }
    }
    
void Tab_widget::load_current_action()
    {
    check_action_type();
    if( current_type == NONE ) // info, global settings
        {
        static_cast< Gestures_settings_tab* >( pages[ TAB_GESTURES_SETTINGS ] )->read_data(); // loads
        static_cast< General_settings_tab* >( pages[ TAB_GENERAL_SETTINGS ] )->read_data(); // loads
        }
    else if( current_type == GROUP )
        {
        static_cast< Action_group_tab* >( pages[ TAB_GROUP_GENERAL ] )->set_data( 
            static_cast< Action_data_group* >( module->current_action_data()));
        static_cast< Condition_list_tab* >( pages[ TAB_CONDITIONS ] )
            ->set_data( module->current_action_data()->conditions());
        }
    else if( current_type == DATA )
        {
        Action_data* tmp = static_cast< Action_data* >( module->current_action_data());
        switch( current_data_type )
            {
            case TYPE_GENERIC:
                kDebug( 1217 ) << "loading unknown" << endl;
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( tmp );
                static_cast< Condition_list_tab* >( pages[ TAB_CONDITIONS ] )
                    ->set_data( tmp->conditions());
                static_cast< Triggers_tab* >( pages[ TAB_TRIGGERS ] )->set_data( tmp->triggers());
                static_cast< Action_list_tab* >( pages[ TAB_ACTIONS ] )->set_data(
                    tmp->actions());
              break;
            case TYPE_COMMAND_URL_SHORTCUT:
                {
                kDebug( 1217 ) << "loading command_url_shortcut" << endl;
                Command_url_shortcut_action_data* item
                    = static_cast< Command_url_shortcut_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->set_data( item->trigger());
                static_cast< Command_url_tab* >( pages[ TAB_COMMAND_URL ] )->set_data( 
                    item->action());
              break;
                }
            case TYPE_MENUENTRY_SHORTCUT:
                {
                kDebug( 1217 ) << "loading menuentry_shortcut" << endl;
                Menuentry_shortcut_action_data* item
                    = static_cast< Menuentry_shortcut_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->set_data( item->trigger());
                static_cast< Menuentry_tab* >( pages[ TAB_MENUENTRY ] )->set_data(
                    item->action());
              break;
                }
            case TYPE_DCOP_SHORTCUT:
                {
                kDebug( 1217 ) << "loading dcop_shortcut" << endl;
                Dcop_shortcut_action_data* item
                    = static_cast< Dcop_shortcut_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->set_data( item->trigger());
                static_cast< Dcop_tab* >( pages[ TAB_DCOP ] )->set_data(
                    item->action());
              break;
                }
            case TYPE_KEYBOARD_INPUT_SHORTCUT:
                {
                kDebug( 1217 ) << "loading keyboard_input_shortcut" << endl;
                Keyboard_input_shortcut_action_data* item
                    = static_cast< Keyboard_input_shortcut_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->set_data( item->trigger());
                static_cast< Keyboard_input_tab* >( pages[ TAB_KEYBOARD_INPUT ] )->set_data(
                    item->action());
              break;
                }
            case TYPE_KEYBOARD_INPUT_GESTURE:
                {
                kDebug( 1217 ) << "loading keyboard_input_gesture" << endl;
                Keyboard_input_gesture_action_data* item
                    = static_cast< Keyboard_input_gesture_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Gesture_triggers_tab* >( pages[ TAB_GESTURE_TRIGGER ] )
                    ->set_data( item->triggers());
                static_cast< Keyboard_input_tab* >( pages[ TAB_KEYBOARD_INPUT ] )->set_data(
                    item->action());
              break;
                }
            case TYPE_ACTIVATE_WINDOW_SHORTCUT:
                {
                kDebug( 1217 ) << "loading activate_window_shortcut" << endl;
                Activate_window_shortcut_action_data* item
                    = static_cast< Activate_window_shortcut_action_data* >( tmp );
                static_cast< General_tab* >( pages[ TAB_GENERAL ] )->set_data( item );
                static_cast< Shortcut_trigger_tab* >( pages[ TAB_SHORTCUT_TRIGGER ] )
                    ->set_data( item->trigger());
                static_cast< Windowdef_list_tab* >( pages[ TAB_WINDOW ] )->set_data(
                    item->action()->window());
              break;
                }
            case TYPE_END:
              assert( false );
            }
        }
    }

void Tab_widget::check_action_type()
    {
    if( module->current_action_data() == NULL )
        {
        kDebug( 1217 ) << "setting none" << endl;
        if( current_type == NONE )
            return;
        show_pages(( TAB_INFO, TAB_GENERAL_SETTINGS, TAB_GESTURES_SETTINGS ));
        current_type = NONE;
        return;
        }
    if( dynamic_cast< Action_data_group* >( module->current_action_data()) != NULL )
        {  // Action_data_group
        kDebug( 1217 ) << "setting group" << endl;
        if( current_type == GROUP )
            return;
        show_pages(( TAB_GROUP_GENERAL, TAB_CONDITIONS ));
        current_type = GROUP;
        return;
        }
    else // standard action
        {
        action_type_t is_type =
            type( static_cast< Action_data* >( module->current_action_data()));
        kDebug( 1217 ) << "setting data " << is_type << endl;
        if( current_type == DATA && is_type == current_data_type )
            return;
        current_type = DATA;
        set_action_type( is_type, true );
        return;
        }
    }
    
void Tab_widget::set_action_type( action_type_t type_P, bool force_P )
    {
    assert( current_type == DATA );
    if( current_data_type == type_P && !force_P )
        return;
    current_data_type = type_P;
    switch( type_P )
        {
        case TYPE_GENERIC:
            show_pages(( TAB_GENERAL, TAB_TRIGGERS, TAB_ACTIONS, TAB_CONDITIONS ));
          break;
        case TYPE_COMMAND_URL_SHORTCUT:
            show_pages(( TAB_GENERAL, TAB_SHORTCUT_TRIGGER, TAB_COMMAND_URL ));
          break;
        case TYPE_MENUENTRY_SHORTCUT:
            show_pages(( TAB_GENERAL, TAB_SHORTCUT_TRIGGER, TAB_MENUENTRY ));
          break;
        case TYPE_DCOP_SHORTCUT:
            show_pages(( TAB_GENERAL, TAB_SHORTCUT_TRIGGER, TAB_DCOP ));
          break;
        case TYPE_KEYBOARD_INPUT_SHORTCUT:
            show_pages(( TAB_GENERAL, TAB_SHORTCUT_TRIGGER, TAB_KEYBOARD_INPUT ));
          break;
        case TYPE_KEYBOARD_INPUT_GESTURE:
            show_pages(( TAB_GENERAL, TAB_GESTURE_TRIGGER, TAB_KEYBOARD_INPUT ));
          break;
        case TYPE_ACTIVATE_WINDOW_SHORTCUT:
            show_pages(( TAB_GENERAL, TAB_SHORTCUT_TRIGGER, TAB_WINDOW ));
          break;
        case TYPE_END:
          assert( false );
        }
    }

void Tab_widget::set_action_type_slot( int type_P )
    {
    set_action_type( static_cast< action_type_t >( type_P ));
    }

const char* const Tab_widget::tab_labels[ Tab_widget::TAB_END ] = {
    I18N_NOOP( "Info" ),  // TAB_INFO
    I18N_NOOP( "General Settings" ), // TAB_GENERAL_SETTINGS
    I18N_NOOP( "Gestures Settings" ), // TAB_GESTURES_SETTINGS
    I18N_NOOP( "General" ), // TAB_GENERAL
    I18N_NOOP( "General" ), // TAB_GROUP_GENERAL
    I18N_NOOP( "Triggers" ), // TAB_TRIGGERS
    I18N_NOOP( "Keyboard Shortcut" ), // TAB_SHORTCUT_TRIGGER
    I18N_NOOP( "Gestures" ), // TAB_GESTURE_TRIGGER
    I18N_NOOP( "Actions" ), // TAB_ACTIONS
    I18N_NOOP( "Command/URL Settings" ), // TAB_COMMAND_URL
    I18N_NOOP( "Menu Entry Settings" ), // TAB_MENUENTRY
    I18N_NOOP( "DCOP Call Settings" ), // TAB_DCOP
    I18N_NOOP( "Keyboard Input Settings" ), // TAB_KEYBOARD_INPUT
    I18N_NOOP( "Window" ), // TAB_WINDOW
    I18N_NOOP( "Conditions" ) // TAB_CONDITIONS
    };

void Tab_widget::show_pages( const Pages_set& pages_P )
    {
    hide(); // this seems to be necessary, otherwise it's not repainter properly
    for( tab_pos_t i = TAB_FIRST;
         i < TAB_END;
         ++i )
        {
        removePage( pages[ i ] );
        if( pages_P.is_set( i )) // don't clear page contents if it stays visible
            disconnect( this, SIGNAL( clear_pages_signal()), pages[ i ], SLOT( clear_data()));
        }
    clear_pages();
    // reconnect all pages to this signal
    disconnect( this, SIGNAL( clear_pages_signal()), NULL, NULL );
    for( tab_pos_t i = TAB_FIRST;
         i < TAB_END;
         ++i )
        {
        if( pages_P.is_set( i ))
            addTab( pages[ i ], i18n( tab_labels[ i ] ));
        connect( this, SIGNAL( clear_pages_signal()), pages[ i ], SLOT( clear_data()));
        }
    show();
    }

Tab_widget::action_type_t Tab_widget::type( const Action_data* data_P )
    {
    Tab_widget::action_type_t ret = Tab_widget::TYPE_GENERIC;
    if( typeid( *data_P ) == typeid( Generic_action_data ))
        ret = Tab_widget::TYPE_GENERIC;
    else if( typeid( *data_P ) == typeid( Command_url_shortcut_action_data ))
        ret = Tab_widget::TYPE_COMMAND_URL_SHORTCUT;
    else if( typeid( *data_P ) == typeid( Menuentry_shortcut_action_data ))
        ret = Tab_widget::TYPE_MENUENTRY_SHORTCUT;
    else if( typeid( *data_P ) == typeid( Dcop_shortcut_action_data ))
        ret = Tab_widget::TYPE_DCOP_SHORTCUT;
    else if( typeid( *data_P ) == typeid( Keyboard_input_shortcut_action_data ))
        ret = Tab_widget::TYPE_KEYBOARD_INPUT_SHORTCUT;
    else if( typeid( *data_P ) == typeid( Keyboard_input_gesture_action_data ))
        ret = Tab_widget::TYPE_KEYBOARD_INPUT_GESTURE;
    else if( typeid( *data_P ) == typeid( Activate_window_shortcut_action_data ))
        ret = Tab_widget::TYPE_ACTIVATE_WINDOW_SHORTCUT;
    else
        assert( false );
    return ret;
    }

} // namespace KHotKeys

#include "tab_widget.moc"
