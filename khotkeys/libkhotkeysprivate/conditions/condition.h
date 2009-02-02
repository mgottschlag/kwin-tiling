#ifndef CONDITION_H
#define CONDITION_H
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

#include <QtCore/QObject>
#include <QtCore/QString>

#include "kdemacros.h"

class KConfigGroup;


namespace KHotKeys {

class ConditionsVisitor;
class Condition_list_base;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT Condition
    {
    Q_DISABLE_COPY( Condition )

    public:
        Condition( Condition_list_base* parent_P = NULL );
        Condition( KConfigGroup& cfg_P, Condition_list_base* parent_P );
        virtual ~Condition();
        virtual bool match() const = 0;
        virtual void updated() const; // called when the condition changes
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;
        virtual const QString description() const = 0;
        virtual Condition* copy() const = 0;
        const Condition_list_base* parent() const;
        Condition_list_base* parent();
        static Condition* create_cfg_read( KConfigGroup& cfg_P, Condition_list_base* parent_P );

        // Reparent the condition to another list.
        void reparent(Condition_list_base *parent);

        virtual void visit( ConditionsVisitor *visitor );

    protected:

        Condition_list_base* _parent;
    };


} // namespace KHotKeys

#endif /* #ifndef CONDITION_H */
