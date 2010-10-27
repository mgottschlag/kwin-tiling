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
          ,storage_id()
    {
    ui.setupUi(this);

    connect(
        ui.applicationButton, SIGNAL(clicked()),
        this, SLOT(selectApplicationClicked()) );

    connect(
        ui.application, SIGNAL(textChanged(QString)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.application, "application" );
    }


MenuentryActionWidget::~MenuentryActionWidget()
    {}


KHotKeys::MenuEntryAction *MenuentryActionWidget::action()
    {
    Q_ASSERT(dynamic_cast<KHotKeys::MenuEntryAction*>(_action));
    return static_cast<KHotKeys::MenuEntryAction*>(_action);
    }


const KHotKeys::MenuEntryAction *MenuentryActionWidget::action() const
    {
    Q_ASSERT(dynamic_cast<KHotKeys::MenuEntryAction*>(_action));
    return static_cast<const KHotKeys::MenuEntryAction*>(_action);
    }


void MenuentryActionWidget::doCopyFromObject()
    {
    Q_ASSERT(action());
    KService::Ptr service = action()->service();

    if (service)
        {
        ui.application->setText( service->name() );
        storage_id = service->storageId();
        }
    else
        {
        ui.application->setText(QString());
        storage_id = QString();
        }
    }


void MenuentryActionWidget::doCopyToObject()
    {
    Q_ASSERT(action());
    action()->set_service( KService::serviceByStorageId(storage_id));
    }


bool MenuentryActionWidget::isChanged() const
    {
    Q_ASSERT(action());

    bool changed;

    // There could be no service set, so be careful!
    if (action()->service())
        {
        changed = ui.application->text() != action()->service()->name();
        }
    else
        {
        // No service set. If the string is not empty something changed.
        changed = ! ui.application->text().isEmpty();
        }

    return changed;
    }


void MenuentryActionWidget::selectApplicationClicked()
    {
    KOpenWithDialog dlg;
    dlg.exec();

    KService::Ptr service = dlg.service();

    if (service)
        {
        ui.application->setText( service->name() );
        storage_id = service->storageId();
        }
    }


#include "moc_menuentry_action_widget.cpp"
