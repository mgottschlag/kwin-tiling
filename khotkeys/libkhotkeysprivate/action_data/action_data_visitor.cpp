/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "action_data/action_data_visitor.h"

#include "action_data/action_data_base.h"

#include <KDE/KDebug>

namespace KHotKeys {

ActionDataVisitor::ActionDataVisitor()
    {}


ActionDataVisitor::~ActionDataVisitor()
    {}


void ActionDataVisitor::visitActionDataBase(ActionDataBase *action)
    {
    kDebug() << action->name();
    Q_ASSERT(false);
    }



ActionDataConstVisitor::ActionDataConstVisitor()
    {}


ActionDataConstVisitor::~ActionDataConstVisitor()
    {}


void ActionDataConstVisitor::visitActionDataBase(const ActionDataBase *action)
    {
    kDebug() << action->name();
    Q_ASSERT(false);
    }

} // KHotKeys



