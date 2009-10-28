/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

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
#include "conditions/conditions_visitor.h"

#include <KDE/KConfig>
#include <KDE/KConfigGroup>

namespace KHotKeys {


Condition_list_base::Condition_list_base( Condition_list_base* parent_P )
    :   Condition( parent_P ), 
        QList< Condition* >()
    {
    }


Condition_list_base::Condition_list_base(
        const QList< Condition* >& children_P,
        Condition_list_base* parent_P)
    : Condition( parent_P ),
      QList< Condition* >( children_P )
    {
    }


Condition_list_base::Condition_list_base( KConfigGroup& cfg_P, Condition_list_base* parent_P )
    : Condition( parent_P )
    {
    int cnt = cfg_P.readEntry( "ConditionsCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        KConfigGroup conditionConfig( cfg_P.config(), cfg_P.name() + QString::number( i ) );
        (void) Condition::create_cfg_read( conditionConfig, this );
        }
    }


Condition_list_base::~Condition_list_base()
    {
    qDeleteAll(*this);
#if 0
    while( !isEmpty())
        {
        Condition* c = first();
        removeAll( c );
        delete c;
        }
#endif
    }


bool Condition_list_base::accepts_children() const
    {
    return true;
    }


void Condition_list_base::append(Condition *cond)
    {
    if (cond->parent() == this)
        {
        return;
        }

    cond->reparent(this);
    QList<Condition*>::append(cond);
    }


Condition_list_base::Iterator Condition_list_base::begin()
    {
    return QList<Condition*>::begin();
    }


Condition_list_base::ConstIterator Condition_list_base::begin() const
    {
    return QList<Condition*>::begin();
    }


void Condition_list_base::clear()
    {
    return QList<Condition*>::clear();
    }


void Condition_list_base::cfg_write( KConfigGroup& cfg_P ) const
    {
    int i = 0;
    for( ConstIterator it(begin());
         it != end();
         ++it, ++i )
        {
        KConfigGroup conditionConfig( cfg_P.config(), cfg_P.name() + QString::number( i ) );
        (*it)->cfg_write( conditionConfig );
        }
    cfg_P.writeEntry( "ConditionsCount", i );
    }


int Condition_list_base::count() const
    {
    return QList<Condition*>::count();
    }


Condition* Condition_list_base::first()
    {
    return QList<Condition*>::first();
    }


Condition const* Condition_list_base::first() const
    {
    return QList<Condition*>::first();
    }


Condition_list_base::Iterator Condition_list_base::end()
    {
    return QList<Condition*>::end();
    }


Condition_list_base::ConstIterator Condition_list_base::end() const
    {
    return QList<Condition*>::end();
    }


bool Condition_list_base::isEmpty() const
    {
    return QList<Condition*>::isEmpty();
    }


int Condition_list_base::removeAll(Condition *const &cond)
    {
    return QList<Condition*>::removeAll(cond);
    }


void Condition_list_base::visit( ConditionsVisitor *visitor )
    {
    visitor->visitConditionsListBase(this);
    }


} // namespace KHotKeys

