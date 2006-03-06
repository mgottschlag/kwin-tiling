/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _GENERAL_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "general_tab.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kdebug.h>
#include <ktextedit.h>

#include <actions.h>
#include <action_data.h>

#include "tab_widget.h"
#include "kcmkhotkeys.h"

namespace KHotKeys
{

General_tab::General_tab( QWidget* parent_P, const char* name_P )
    : General_tab_ui( parent_P, name_P )
    {
    for( Tab_widget::action_type_t i = Tab_widget::TYPE_FIRST;
         i < Tab_widget::TYPE_END;
         ++i )
        switch( i )
            {
            case Tab_widget::TYPE_GENERIC:
                action_type_combo->insertItem( i18n( "Generic" ), i );
              break;
            case Tab_widget::TYPE_COMMAND_URL_SHORTCUT:
                action_type_combo->insertItem(
                    i18n( "Keyboard Shortcut -> Command/URL (simple)" ), i );
              break;
            case Tab_widget::TYPE_MENUENTRY_SHORTCUT:
                action_type_combo->insertItem(
                    i18n( "K-Menu Entry (simple)" ), i );
              break;
            case Tab_widget::TYPE_DCOP_SHORTCUT:
                action_type_combo->insertItem(
                    i18n( "Keyboard Shortcut -> DCOP Call (simple)" ), i );
              break;
            case Tab_widget::TYPE_KEYBOARD_INPUT_SHORTCUT:
                action_type_combo->insertItem(
                    i18n( "Keyboard Shortcut -> Keyboard Input (simple)" ), i );
              break;
            case Tab_widget::TYPE_KEYBOARD_INPUT_GESTURE:
                action_type_combo->insertItem(
                    i18n( "Gesture -> Keyboard Input (simple)" ), i );
              break;
            case Tab_widget::TYPE_ACTIVATE_WINDOW_SHORTCUT:
                action_type_combo->insertItem(
                    i18n( "Keyboard Shortcut -> Activate Window (simple)" ), i );
              break;
            case Tab_widget::TYPE_END:
              assert( false );
            }
    clear_data();
    // KHotKeys::Module::changed()
    connect( action_name_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( disable_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( comment_multilineedit, SIGNAL( textChanged()),
        module, SLOT( changed()));
    connect( action_type_combo, SIGNAL( activated( int )),
        module, SLOT( changed()));
    }
    
void General_tab::clear_data()
    {
    disconnect( action_name_lineedit, SIGNAL( textChanged( const QString& )),
        this, SLOT( action_name_changed( const QString& )));
    disconnect( action_type_combo, SIGNAL( activated( int )),
        this, SIGNAL( action_type_changed( int ))); // CHECKME neodpoji to sloty od nej ?
    action_name_lineedit->clear();
    disable_checkbox->setChecked( false );
    disable_checkbox->setText( i18n( "&Disable" ));
    comment_multilineedit->clear();
    action_type_combo->setCurrentIndex( Tab_widget::TYPE_GENERIC );
//    module->set_action_type( data_P->type()); CHECKME tohle asi tady ne
    }

void General_tab::set_data( const Action_data* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    action_name_lineedit->setText( data_P->name());
    disable_checkbox->setChecked( !data_P->enabled( true ));
    if( !data_P->parent()->enabled( false ))
        disable_checkbox->setText( i18n( "&Disable (group is disabled)" ));
    else
        disable_checkbox->setText( i18n( "&Disable" ));
    comment_multilineedit->setText( data_P->comment());
    action_type_combo->setCurrentIndex( Tab_widget::type( data_P ));
//    module->set_action_type( data_P->type());
    connect( action_name_lineedit, SIGNAL( textChanged( const QString& )),
        SLOT( action_name_changed( const QString& )));
    connect( action_type_combo, SIGNAL( activated( int )),
        SIGNAL( action_type_changed( int )));
    }

void General_tab::get_data( QString& name_O, QString& comment_O, bool& enabled_O )
    {
    name_O = action_name_lineedit->text();
    comment_O = comment_multilineedit->text();
    enabled_O = !disable_checkbox->isChecked();
    }
    
void General_tab::action_name_changed( const QString& name_P )
    {
    module->action_name_changed( name_P );
    }
    
} // namespace KHotKeys

#include "general_tab.moc"
