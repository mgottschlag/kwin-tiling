/* Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "conditions/conditions_visitor.h"
#include "conditions/conditions_list_base.h"

namespace KHotKeys {

ConditionsVisitor::ConditionsVisitor( bool recurse )
    :   _recurse(recurse)
    {}


ConditionsVisitor::~ConditionsVisitor()
    {}


} // namespace KHotKeys
