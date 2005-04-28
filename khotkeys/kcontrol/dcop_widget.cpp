/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _DCOP_WIDGET_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "dcop_widget.h"

#include <qlineedit.h>
#include <qpushbutton.h>

#include <krun.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <actions.h>

#include "kcmkhotkeys.h"

namespace KHotKeys
{

Dcop_widget::Dcop_widget( QWidget* parent_P, const char* name_P )
    : Dcop_widget_ui( parent_P, name_P )
    {
    clear_data();
    try_button->setText( i18n( "to try", "&Try" )); // Qt designer can't do this
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
    
void Dcop_widget::clear_data()
    {
    remote_app_lineedit->clear();
    remote_object_lineedit->clear();
    called_function_lineedit->clear();
    arguments_lineedit->clear();
    // CHECKME nebo spis multilineedit ?
    }

void Dcop_widget::set_data( const Dcop_action* data_P )
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

Dcop_action* Dcop_widget::get_data( Action_data* data_P ) const
    {
    return new Dcop_action( data_P, remote_app_lineedit->text().stripWhiteSpace(),
        remote_object_lineedit->text().stripWhiteSpace(),
        called_function_lineedit->text().stripWhiteSpace(), arguments_lineedit->text());
    }
    
// CHECKME later "steal" whole interfaces browsing from kdcop
void Dcop_widget::run_kdcop_pressed()
    {
    if( KRun::runCommand( "kdcop" ) == 0 )
        KMessageBox::sorry( NULL, i18n( "Failed to run KDCOP" ));
    }
    
void Dcop_widget::try_pressed()
    {
    Dcop_action* tmp = get_data( NULL ); // CHECKME
    tmp->execute();
    delete tmp;
    }
    
} // namespace KHotKeys

#include "dcop_widget.moc"
