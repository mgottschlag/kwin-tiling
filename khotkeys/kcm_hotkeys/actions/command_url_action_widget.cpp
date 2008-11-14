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

#include "command_url_action_widget.h"

#include <KDE/KLineEdit>


CommandUrlActionWidget::CommandUrlActionWidget(
    KHotKeys::CommandUrlAction *action,
    QWidget *parent )
        : Base( action, parent )
    {
    ui.setupUi(this);

    connect(
        ui.command, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.command, "command" );
    }


CommandUrlActionWidget::~CommandUrlActionWidget()
    {
    }


KHotKeys::CommandUrlAction *CommandUrlActionWidget::action()
    {
    return static_cast<KHotKeys::CommandUrlAction*>(_action);
    }


const KHotKeys::CommandUrlAction *CommandUrlActionWidget::action() const
    {
    return static_cast<const KHotKeys::CommandUrlAction*>(_action);
    }


void CommandUrlActionWidget::doCopyFromObject()
    {
    Q_ASSERT(action());
    ui.command->lineEdit()->setText( action()->command_url() );
    }


void CommandUrlActionWidget::doCopyToObject()
    {
    Q_ASSERT(action());
    action()->set_command_url( ui.command->lineEdit()->text() );
    }


bool CommandUrlActionWidget::isChanged() const
    {
    Q_ASSERT(action());
    return action()->command_url() != ui.command->lineEdit()->text();
    }


#include "moc_command_url_action_widget.cpp"
