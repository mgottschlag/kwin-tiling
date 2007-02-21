/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _MENUENTRY_WIDGET_CPP_



#include "menuentry_widget.h"

#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <q3groupbox.h>

#include <kdebug.h>

#include <actions.h>
#include <action_data.h>

#include "windowdef_list_widget.h"
#include "kcmkhotkeys.h"

namespace KHotKeys
{

Menuentry_widget::Menuentry_widget( QWidget* parent_P, const char* name_P )
    : Menuentry_widget_ui( parent_P, name_P )
    {
    clear_data();
    // KHotKeys::Module::changed()
    connect( menuentry_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    }

void Menuentry_widget::clear_data()
    {
    menuentry_lineedit->clear();
    }

void Menuentry_widget::set_data( const Menuentry_action* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    menuentry_lineedit->setText( data_P->command_url());
    }

Menuentry_action* Menuentry_widget::get_data( Action_data* data_P ) const
    {
    return new Menuentry_action( data_P, menuentry_lineedit->text());
    }

void Menuentry_widget::browse_pressed()
    { // CHECKME TODO
    }
    
} // namespace KHotKeys

#include "menuentry_widget.moc"
