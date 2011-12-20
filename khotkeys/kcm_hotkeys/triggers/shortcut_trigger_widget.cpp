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

#include "action_data/action_data.h"
#include "triggers/triggers.h"

#include <QtGui/QKeySequence>

#include <KDE/KDebug>


ShortcutTriggerWidget::ShortcutTriggerWidget( KHotKeys::ShortcutTrigger *trigger, QWidget *parent )
    : TriggerWidgetBase(trigger, parent)
    {
    shortcut_trigger_ui.setupUi(this);

    shortcut_trigger_ui.shortcut->setCheckForConflictsAgainst(
        // Don't know why that is necessary but it doesn't compile
        // without.
        KKeySequenceWidget::ShortcutTypes(
            KKeySequenceWidget::GlobalShortcuts
                | KKeySequenceWidget::StandardShortcuts ));

    connect(
        shortcut_trigger_ui.shortcut, SIGNAL(keySequenceChanged(QKeySequence)),
        _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(shortcut_trigger_ui.shortcut, "shortcut" );

    // If the global shortcuts is changed outside of the dialog just copy the
    // new key sequencence. It doesn't matter if the user changed the sequence
    // here.
    connect(
        trigger, SIGNAL(globalShortcutChanged(QKeySequence)),
        this, SLOT(_k_globalShortcutChanged(QKeySequence)) );
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
    shortcut_trigger_ui.shortcut->setKeySequence( trigger()->shortcut().primary() );
    }


void ShortcutTriggerWidget::doCopyToObject()
    {
    Q_ASSERT(trigger());
    trigger()->set_key_sequence( shortcut_trigger_ui.shortcut->keySequence());
    }


bool ShortcutTriggerWidget::isChanged() const
    {
    Q_ASSERT(trigger());
    return trigger()->shortcut().primary() != shortcut_trigger_ui.shortcut->keySequence();
    }


void ShortcutTriggerWidget::_k_globalShortcutChanged(const QKeySequence &seq)
    {
    shortcut_trigger_ui.shortcut->setKeySequence(seq);
    }


#include "moc_shortcut_trigger_widget.cpp"
