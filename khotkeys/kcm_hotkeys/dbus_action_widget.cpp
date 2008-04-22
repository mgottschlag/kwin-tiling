/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "dbus_action_widget.h"

#include <KDE/KMessageBox>
#include <KDE/KRun>

DbusActionWidget::DbusActionWidget(
    KHotKeys::DBusAction *action,
    QWidget *parent )
        : Base(action, parent)
    {
    ui.setupUi(this);

    connect(
        ui.application, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.application, "application" );
    connect(
        ui.object, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.object, "object" );
    connect(
        ui.function, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.function, "function" );
    connect(
        ui.arguments, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.arguments, "arguments" );

    connect(
        ui.launchButton, SIGNAL(clicked()),
        this, SLOT(launchDbusBrowser()) );
    connect(
        ui.execButton, SIGNAL(clicked()),
        this, SLOT(execCommand()) );
    }


DbusActionWidget::~DbusActionWidget()
    {
    }


KHotKeys::DBusAction *DbusActionWidget::action()
    {
    return static_cast<KHotKeys::DBusAction*>(_action);
    }


const KHotKeys::DBusAction *DbusActionWidget::action() const
    {
    return static_cast<const KHotKeys::DBusAction*>(_action);
    }


void DbusActionWidget::doCopyFromObject()
    {
    Q_ASSERT(action());
    ui.application->setText( action()->remote_application() );
    ui.object->setText( action()->remote_object() );
    ui.function->setText( action()->called_function() );
    ui.arguments->setText( action()->arguments() );
    }


void DbusActionWidget::doCopyToObject()
    {
    Q_ASSERT(action());
    action()->set_remote_application( ui.application->text() );
    action()->set_remote_object( ui.object->text() );
    action()->set_called_function( ui.function->text() );
    action()->set_arguments( ui.arguments->text() );
    }


void DbusActionWidget::execCommand() const
    {
    KHotKeys::DBusAction action(
        0,
        ui.application->text(),
        ui.object->text(),
        ui.function->text(),
        ui.arguments->text() );

    // TODO: Error handling
    action.execute();
    }


bool DbusActionWidget::isChanged() const
    {
    Q_ASSERT(action());
    return ui.application->text() != action()->remote_application()
        || ui.object->text()      != action()->remote_object()
        || ui.function->text()    != action()->called_function()
        || ui.arguments->text()   != action()->arguments();
    }


void DbusActionWidget::launchDbusBrowser() const
    {
    if( KRun::runCommand( "qdbusviewer", window()) == 0 )
        {
        KMessageBox::sorry( window(), i18n( "Failed to run qdbusviewer" ));
        }
    }


#include "moc_dbus_action_widget.cpp"
