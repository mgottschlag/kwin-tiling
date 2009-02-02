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

#include "window_trigger_widget.h"
#include "helper_widgets/window_definition_list_widget.h"
#include "windows_helper/window_selection_list.h"
#include "windows_helper/window_selection_rules.h"

#include "windows_handler.h"

#include "kdebug.h"


WindowTriggerWidget::WindowTriggerWidget( KHotKeys::WindowTrigger *trigger, QWidget *parent )
    :   TriggerWidgetBase(trigger, parent)
        ,_windowdef_widget(NULL)
    {
    window_trigger_ui.setupUi(this);

    if (trigger->windows()->count() == 0)
        {
        trigger->windows()->insert(0, new KHotKeys::Windowdef_simple("", ""));
        }

    QHBoxLayout *layout = new QHBoxLayout();
    _windowdef_widget = new WindowDefinitionListWidget(trigger->windows(), window_trigger_ui.window_group);
    layout->addWidget(_windowdef_widget);
    window_trigger_ui.window_group->setLayout(layout);

    connect(window_trigger_ui.window_appears, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()));
    _changedSignals->setMapping(window_trigger_ui.window_appears, "appears");
    connect(window_trigger_ui.window_disappears, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()));
    _changedSignals->setMapping(window_trigger_ui.window_disappears, "disappears");
    connect(window_trigger_ui.window_gets_focus, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()));
    _changedSignals->setMapping(window_trigger_ui.window_gets_focus, "gets focus");
    connect(window_trigger_ui.window_lost_focus, SIGNAL(toggled(bool)),
            _changedSignals, SLOT(map()));
    _changedSignals->setMapping(window_trigger_ui.window_lost_focus, "lost focus");

    connect(_windowdef_widget, SIGNAL(changed(bool)),
            SLOT(slotWindowSelectionChanged(bool)));
    }


WindowTriggerWidget::~WindowTriggerWidget()
    {
    }


KHotKeys::WindowTrigger *WindowTriggerWidget::trigger()
    {
    return static_cast<KHotKeys::WindowTrigger*>(_trigger);
    }


const KHotKeys::WindowTrigger *WindowTriggerWidget::trigger() const
    {
    return static_cast<const KHotKeys::WindowTrigger*>(_trigger);
    }


void WindowTriggerWidget::doCopyFromObject()
    {
    Q_ASSERT(trigger());
    _windowdef_widget->copyFromObject();

    window_trigger_ui.window_appears->setChecked(
            trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_APPEARS));
    window_trigger_ui.window_disappears->setChecked(
            trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_DISAPPEARS));
    window_trigger_ui.window_gets_focus->setChecked(
            trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_ACTIVATES));
    window_trigger_ui.window_lost_focus->setChecked(
            trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_DEACTIVATES));
    }


void WindowTriggerWidget::doCopyToObject()
    {
    Q_ASSERT(trigger());
    _windowdef_widget->copyToObject();

    KHotKeys::WindowTrigger::WindowEvents events;
    if (window_trigger_ui.window_appears->isChecked())
        events |= KHotKeys::WindowTrigger::WINDOW_APPEARS;
    if (window_trigger_ui.window_disappears->isChecked())
        events |= KHotKeys::WindowTrigger::WINDOW_DISAPPEARS;
    if (window_trigger_ui.window_gets_focus->isChecked())
        events |= KHotKeys::WindowTrigger::WINDOW_ACTIVATES;
    if (window_trigger_ui.window_lost_focus->isChecked())
        events |= KHotKeys::WindowTrigger::WINDOW_DEACTIVATES;

    trigger()->setOnWindowEvents(events);
    }


bool WindowTriggerWidget::isChanged() const
    {
    Q_ASSERT(trigger());

    if(     window_trigger_ui.window_appears->isChecked()
                != trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_APPEARS)
        ||  window_trigger_ui.window_disappears->isChecked()
                != trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_DISAPPEARS)
        ||  window_trigger_ui.window_gets_focus->isChecked()
                != trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_ACTIVATES)
        ||  window_trigger_ui.window_lost_focus->isChecked()
                != trigger()->triggers_on(KHotKeys::WindowTrigger::WINDOW_DEACTIVATES))
        return true;

    return _windowdef_widget->isChanged();
    }


void WindowTriggerWidget::slotWindowSelectionChanged(bool isChanged) const
    {
    emit changed(isChanged);
    }



#include "moc_window_trigger_widget.cpp"
