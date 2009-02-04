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

#include "action_group_widget.h"

#include "conditions/conditions_widget.h"

#include "action_data/action_data_group.h"

ActionGroupWidget::ActionGroupWidget( QWidget *parent )
    :   HotkeysWidgetBase(parent)
        ,_conditions(new ConditionsWidget)
    {
    extend(_conditions, i18n("Conditions"));
    connect(_conditions, SIGNAL(changed(bool)),
            SLOT(slotChanged()));
    }


ActionGroupWidget::~ActionGroupWidget()
    {
    _conditions = NULL;
    }


void ActionGroupWidget::setActionData(KHotKeys::ActionDataGroup *group)
    {
    _data = group;

    // BUG: conditions copies twice from the original. Once in
    // setConditionsList and once because of the copyFromObject call below.
    Q_ASSERT(_conditions);
    _conditions->setConditionsList(group->conditions());

    Base::copyFromObject();
    }


void ActionGroupWidget::doCopyFromObject()
    {
    Q_ASSERT(data());
    Base::doCopyFromObject();

    Q_ASSERT(_conditions);
    _conditions->copyFromObject();
    }


void ActionGroupWidget::doCopyToObject()
    {
    Q_ASSERT(data());
    Base::doCopyToObject();

    Q_ASSERT(_conditions);
    _conditions->copyToObject();
    }


bool ActionGroupWidget::isChanged() const
    {
    return _conditions->hasChanges() || Base::isChanged();
    }


#include "moc_action_group_widget.cpp"
