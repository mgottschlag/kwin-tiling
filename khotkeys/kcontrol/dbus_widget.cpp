/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _DBUS_WIDGET_CPP_



#include "dbus_widget.h"

#include <QLineEdit>
#include <QPushButton>

#include <krun.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <actions.h>

#include "kcmkhotkeys.h"

namespace KHotKeys
{

Dbus_widget::Dbus_widget( QWidget* parent_P, const char* name_P )
    : Dbus_widget_ui( parent_P, name_P )
    {
    clear_data();
    try_button->setText( i18nc( "to try", "&Try" )); // Qt designer can't do this
    // KHotKeys::Module::changed()
    connect( remote_app_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( remote_object_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( called_function_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    connect( arguments_lineedit, SIGNAL( textChanged( const QString& )),
        module, SLOT( changed()));
    }
    
void Dbus_widget::clear_data()
    {
    remote_app_lineedit->clear();
    remote_object_lineedit->clear();
    called_function_lineedit->clear();
    arguments_lineedit->clear();
    // CHECKME nebo spis multilineedit ?
    }

void Dbus_widget::set_data( const Dbus_action* data_P )
    {
    if( data_P == NULL )
        {
        clear_data();
        return;
        }
    remote_app_lineedit->setText( data_P->remote_application());
    remote_object_lineedit->setText( data_P->remote_object());
    called_function_lineedit->setText( data_P->called_function());
    arguments_lineedit->setText( data_P->arguments());
    // CHECKME nebo spis multilineedit ?
    }

Dbus_action* Dbus_widget::get_data( Action_data* data_P ) const
    {
    return new Dbus_action( data_P, remote_app_lineedit->text().trimmed(),
        remote_object_lineedit->text().trimmed(),
        called_function_lineedit->text().trimmed(), arguments_lineedit->text());
    }
    
// CHECKME later "steal" whole interfaces browsing from dbus browser
void Dbus_widget::run_dbus_browser_pressed()
    {
    if( KRun::runCommand( "qdbusviewer", topLevelWidget()) == 0 )
        KMessageBox::sorry( topLevelWidget(), i18n( "Failed to run qdbusviewer" ));
    }
    
void Dbus_widget::try_pressed()
    {
    Dbus_action* tmp = get_data( NULL ); // CHECKME
    tmp->execute();
    delete tmp;
    }
    
} // namespace KHotKeys

#include "dbus_widget.moc"
