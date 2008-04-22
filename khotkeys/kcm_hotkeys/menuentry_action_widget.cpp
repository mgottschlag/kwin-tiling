/*
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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

#include "menuentry_action_widget.h"

#include <KDE/KDebug>
#include <KDE/KOpenWithDialog>



MenuentryActionWidget::MenuentryActionWidget( KHotKeys::MenuEntryAction *action, QWidget *parent )
        : ActionWidgetBase(action, parent )
    {
    ui.setupUi(this);

    connect(
        ui.applicationButton, SIGNAL(clicked()),
        this, SLOT(selectApplicationClicked()) );
    }


MenuentryActionWidget::~MenuentryActionWidget()
    {}


KHotKeys::MenuEntryAction *MenuentryActionWidget::action()
    {
    return static_cast<KHotKeys::MenuEntryAction*>(_action);
    }


const KHotKeys::MenuEntryAction *MenuentryActionWidget::action() const
    {
    return static_cast<const KHotKeys::MenuEntryAction*>(_action);
    }


void MenuentryActionWidget::doCopyFromObject()
    {
    Q_ASSERT(action());
    KService::Ptr service = action()->service();

    if (service)
        {
        ui.application->setText( service->name() );
        }
    else
        {
        ui.application->setText("");
        }
    }


void MenuentryActionWidget::doCopyToObject()
    {
    Q_ASSERT(action());

    action()->set_service( KService::serviceByName( ui.application->text() ));
    }


bool MenuentryActionWidget::isChanged() const
    {
    Q_ASSERT(action());
    // TODO
    return true;
    }


void MenuentryActionWidget::selectApplicationClicked()
    {
    KOpenWithDialog dlg;
    dlg.exec();

    KService::Ptr service = dlg.service();

    if (service)
        {
        ui.application->setText( service->name() );
        }
    }


#include "moc_menuentry_action_widget.cpp"
