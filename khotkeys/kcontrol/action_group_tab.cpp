/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _ACTION_GROUP_TAB_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "action_group_tab.h"

#include <QLineEdit>
#include <QCheckBox>

#include <klocale.h>
#include <kmessagebox.h>
#include <ktextedit.h>

#include <actions.h>
#include <action_data.h>

#include "kcmkhotkeys.h"

namespace KHotKeys
{

Action_group_tab::Action_group_tab( QWidget* parent_P , const char* name_P )
    : Action_group_tab_ui( parent_P, name_P )
    {
    clear_data();
    // KHotKeys::Module::changed()
    connect( action_name_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( disable_checkbox, SIGNAL( clicked()),
        module, SLOT( changed()));
    connect( comment_multilineedit, SIGNAL( textChanged()),
        module, SLOT( changed()));
    }
    
void Action_group_tab::clear_data()
    {
    disconnect( action_name_lineedit, SIGNAL( textChanged( const QString& )), this,
        SLOT( action_group_name_changed( const QString& )));
    action_name_lineedit->clear();
    action_name_lineedit->setReadOnly( false );
    disable_checkbox->setChecked( false );
    disable_checkbox->setText( i18n( "&Disable" ));
    comment_multilineedit->clear();
    system_group = Action_data_group::SYSTEM_NONE;
    }

void Action_group_tab::set_data( const Action_data_group* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    action_name_lineedit->setText( data_P->name());
    action_name_lineedit->setReadOnly( data_P->is_system_group());
    disable_checkbox->setChecked( !data_P->enabled( true ));
    if( !data_P->parent()->enabled( false ))
        disable_checkbox->setText( i18n( "&Disable (group is disabled)" ));
    else
        disable_checkbox->setText( i18n( "&Disable" ));
    comment_multilineedit->setPlainText( data_P->comment());
    connect( action_name_lineedit, SIGNAL( textChanged( const QString& )), this,
        SLOT( action_group_name_changed( const QString& )));
    system_group = data_P->system_group();
    }

Action_data_group* Action_group_tab::get_data( Action_data_group* parent_P,
    Condition_list* conditions_P ) const
    {
    QString name = action_name_lineedit->text();
    return new Action_data_group( parent_P, name, comment_multilineedit->toPlainText(), conditions_P,
        system_group, !disable_checkbox->isChecked());
    }
    
void Action_group_tab::action_group_name_changed( const QString& name_P )
    {
    module->action_name_changed( name_P );
    }

} // namespace KHotKeys

#include "action_group_tab.moc"
