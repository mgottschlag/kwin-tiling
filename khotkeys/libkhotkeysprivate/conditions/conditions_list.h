#ifndef CONDITIONS_LIST_H
#define CONDITIONS_LIST_H
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


#include "conditions/conditions_list_base.h"

#include <QtCore/QString>

class KConfigGroup;

namespace KHotKeys {

class ActionDataBase;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT Condition_list
    : public Condition_list_base
    {
    typedef Condition_list_base base;
    public:
        Condition_list( const QString& comment_P, ActionDataBase* parent = NULL );
        Condition_list( KConfigGroup& cfg_P, ActionDataBase* data_P );
        void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Condition_list* copy() const;
        virtual bool match() const;
        const QString& comment() const;
        void set_data( ActionDataBase* data_P );
        virtual void updated() const;
        virtual const QString description() const;

        virtual void visit(ConditionsVisitor *visitor);
    private:
        QString _comment;
        ActionDataBase* data;
    };


} // namespace KHotKeys

#endif /* #ifndef CONDITIONS_LIST_H */
