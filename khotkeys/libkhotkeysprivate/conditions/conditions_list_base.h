#ifndef CONDITION_LIST_BASE_H
#define CONDITION_LIST_BASE_H
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


#include "conditions/condition.h"

#include "kdemacros.h"

#include <QtCore/QList>


class KConfigGroup;

namespace KHotKeys {

class ConditionsVisitor;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT Condition_list_base : public Condition, private QList < Condition* >
    {
    typedef Condition base;

    public:

        Condition_list_base( Condition_list_base* parent = NULL );

        Condition_list_base(
                const QList< Condition* >& children_P,
                Condition_list_base* parent_P );

        Condition_list_base( KConfigGroup& cfg_P, Condition_list_base* parent_P );

        virtual ~Condition_list_base();

        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual bool accepts_children() const;

        typedef QList< Condition* >::iterator Iterator;
        typedef QList< Condition* >::const_iterator ConstIterator;

        void append(Condition*);

        Iterator begin();
        ConstIterator begin() const;

        Iterator end();
        ConstIterator end() const;

        Condition *first();
        Condition const* first() const;

        int count() const;

        bool isEmpty() const;

        void clear();

        virtual void visit( ConditionsVisitor *visitor );

protected:

        int removeAll(Condition *const &);

        friend class Condition;
    };


} // namespace KHotKeys

#endif /* #ifndef CONDITION_LIST_BASE_H */
