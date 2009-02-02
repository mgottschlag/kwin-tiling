#ifndef CONDITIONS_VISITOR_H
#define CONDITIONS_VISITOR_H
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

#include "kdemacros.h"

namespace KHotKeys {
    class Condition_list_base;
    class Condition_list;
    class Condition;


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT ConditionsVisitor
    {
public:

    ConditionsVisitor( bool recurse = true );
    virtual ~ ConditionsVisitor();

    virtual void visitCondition( Condition *condition ) = 0;
    virtual void visitConditionsList( Condition_list *list) = 0;
    virtual void visitConditionsListBase( Condition_list_base *list) = 0;

private:

    bool _recurse;

    }; // class ConditionsVisitor


} // namespace KHotKeys


#endif /* #ifndef CONDITIONS_VISITOR_H */
