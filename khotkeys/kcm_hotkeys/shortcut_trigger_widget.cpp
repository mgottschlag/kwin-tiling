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

#include "shortcut_trigger_widget.h"

#include "action_data.h"
#include "triggers.h"

#include <QtGui/QKeySequence>

#include <KDE/KDebug>


ShortcutTriggerWidget::ShortcutTriggerWidget( KHotKeys::ShortcutTrigger *trigger, QWidget *parent )
    : TriggerWidgetBase(trigger, parent)
    {
    shortcut_action_ui.setupUi(this);

    // mergeLayouts( ui.gridLayout, shortcut_action_ui.gridLayout );

    connect(
        shortcut_action_ui.shortcut, SIGNAL(keySequenceChanged(QKeySequence)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(shortcut_action_ui.shortcut, "shortcut" );

    copyFromObject();
    }


ShortcutTriggerWidget::~ShortcutTriggerWidget()
    {
    }


KHotKeys::ShortcutTrigger *ShortcutTriggerWidget::trigger()
    {
    return static_cast<KHotKeys::ShortcutTrigger*>(_trigger);
    }


const KHotKeys::ShortcutTrigger *ShortcutTriggerWidget::trigger() const
    {
    return static_cast<const KHotKeys::ShortcutTrigger*>(_trigger);
    }


void ShortcutTriggerWidget::doCopyFromObject()
    {
    Q_ASSERT(trigger());
    shortcut_action_ui.shortcut->setKeySequence( trigger()->shortcut().primary() );
    }


void ShortcutTriggerWidget::doCopyToObject()
    {
    Q_ASSERT(trigger());
    trigger()->set_key_sequence( shortcut_action_ui.shortcut->keySequence());
    }


bool ShortcutTriggerWidget::isChanged() const
    {
    Q_ASSERT(trigger());
    return trigger()->shortcut().primary() != shortcut_action_ui.shortcut->keySequence();
    }


#include "moc_shortcut_trigger_widget.cpp"
