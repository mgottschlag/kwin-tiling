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

#include "simple_action_data_widget.h"

#include "actions/command_url_action_widget.h"
#include "actions/dbus_action_widget.h"
#include "actions/menuentry_action_widget.h"
#include "triggers/shortcut_trigger_widget.h"
#include "triggers/window_trigger_widget.h"

#include <KDE/KDebug>


SimpleActionDataWidget::SimpleActionDataWidget( QWidget *parent )
        : HotkeysWidgetBase( parent )
         ,currentTrigger(NULL)
         ,currentAction(NULL)
    {
    // We add ourself to the layout
    QWidget *widget = new QWidget;
    ui.setupUi(widget);
    widget->layout()->setContentsMargins(0,0,0,0);
    layout()->addWidget(widget);
    }


SimpleActionDataWidget::~SimpleActionDataWidget()
    {
    delete currentTrigger;
    delete currentAction;
    }


bool SimpleActionDataWidget::isChanged() const
    {
    return ( currentTrigger && currentTrigger->isChanged() )
        || ( currentAction && currentAction->isChanged() )
        || Base::isChanged();
    }


void SimpleActionDataWidget::doCopyFromObject()
    {
    Base::doCopyFromObject();

    if (currentTrigger)
        {
        currentTrigger->copyFromObject();
        }

    if (currentAction)
        {
        currentAction->copyFromObject();
        }

    }


void SimpleActionDataWidget::doCopyToObject()
    {
    Base::doCopyToObject();

    if (currentTrigger)
        {
        currentTrigger->copyToObject();
        }

    if (currentAction)
        {
        currentAction->copyToObject();
        }
    }


void SimpleActionDataWidget::setActionData( KHotKeys::SimpleActionData* pData )
    {
    _data = pData;

    // Now go and work on the trigger
    delete currentTrigger; currentTrigger = NULL;

    if ( KHotKeys::Trigger *trg = data()->trigger() )
        {
        switch ( trg->type() )
            {
            case KHotKeys::Trigger::ShortcutTriggerType:
                currentTrigger = new ShortcutTriggerWidget( static_cast<KHotKeys::ShortcutTrigger*>(trg) );
                break;

            case KHotKeys::Trigger::WindowTriggerType:
                currentTrigger = new WindowTriggerWidget( static_cast<KHotKeys::WindowTrigger*>(trg) );
                break;

            default:
                kDebug() << "Unknown trigger type";
            };
        }

    Q_ASSERT( ui.triggerBox->layout() );
    if (currentTrigger )
        {
        kDebug() << "Connecting";
        connect(
            currentTrigger, SIGNAL(changed(bool)),
            this, SLOT(slotChanged() ));
        ui.triggerBox->layout()->addWidget(currentTrigger);
        }

    // Now go and work on the trigger
    delete currentAction; currentAction = NULL;

    if ( KHotKeys::Action *act = data()->action() )
        {
        switch ( act->type() )
            {
            case KHotKeys::Action::MenuEntryActionType:
                currentAction = new MenuentryActionWidget( static_cast<KHotKeys::MenuEntryAction*>(act) );
                break;

            case KHotKeys::Action::DBusActionType:
                currentAction = new DbusActionWidget( static_cast<KHotKeys::DBusAction*>(act) );
                break;

            case KHotKeys::Action::CommandUrlActionType:
                currentAction = new CommandUrlActionWidget( static_cast<KHotKeys::CommandUrlAction*>(act) );
                break;

            default:
                kDebug() << "Unknown action type";
            };
        }

    Q_ASSERT( ui.actionBox->layout() );
    if (currentAction )
        {
        connect(
            currentAction, SIGNAL(changed(bool)),
            this, SLOT(slotChanged() ));
        ui.actionBox->layout()->addWidget(currentAction);
        }

    Base::copyFromObject();
    }



#include "moc_simple_action_data_widget.cpp"
