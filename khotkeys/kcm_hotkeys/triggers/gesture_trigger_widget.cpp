/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>

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

#include "gesture_trigger_widget.h"


GestureTriggerWidget::GestureTriggerWidget(KHotKeys::GestureTrigger *trigger, QWidget *parent)
    : TriggerWidgetBase(trigger, parent)
    {
    ui.setupUi(this);

    connect(ui.gesture, SIGNAL(changed()),
            this, SLOT(slotGestureHasChanged()) );

    connect(ui.gesture, SIGNAL(changed()),
            _changedSignals, SLOT(map()) );
    _changedSignals->setMapping(ui.gesture, "gesture" );

    hasChanged = false;
    }


GestureTriggerWidget::~GestureTriggerWidget()
    {
    }


void GestureTriggerWidget::doCopyFromObject()
    {
    Q_ASSERT(trigger());
    ui.gesture->setPointData(trigger()->pointData(), false);
    hasChanged = false;
    return;
    }


void GestureTriggerWidget::doCopyToObject()
    {
    Q_ASSERT(trigger());
    hasChanged = false;
    trigger()->setPointData(ui.gesture->pointData());
    return;
    }


bool GestureTriggerWidget::isChanged() const
    {
    Q_ASSERT(trigger());
    return hasChanged;
    }

void GestureTriggerWidget::slotGestureHasChanged()
    {
    hasChanged = true;
    }

KHotKeys::GestureTrigger *GestureTriggerWidget::trigger()
    {
    return static_cast<KHotKeys::GestureTrigger*>(_trigger);
    }


const KHotKeys::GestureTrigger *GestureTriggerWidget::trigger() const
    {
    return static_cast<const KHotKeys::GestureTrigger*>(_trigger);
    }


#include "moc_gesture_trigger_widget.cpp"
