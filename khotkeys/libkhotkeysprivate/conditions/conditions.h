/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef CONDITIONS_H
#define CONDITIONS_H

#include "conditions/conditions_list_base.h"

#include "action_data/action_data_group.h"
#include "windows_handler.h"

class KConfigGroup;

namespace KHotKeys
{

class Condition;

class KDE_EXPORT Not_condition
    : public Condition_list_base
    {
    typedef Condition_list_base base;
    public:
        Not_condition( Condition_list_base* parent = NULL );
        Not_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P );
        virtual bool match() const;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Not_condition* copy() const;
        virtual const QString description() const;
        const Condition* condition() const;
        virtual bool accepts_children() const;
    };

class KDE_EXPORT And_condition
    : public Condition_list_base
    {
    typedef Condition_list_base base;
    public:
        And_condition( Condition_list_base* parent = NULL );
        And_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P );
        virtual bool match() const;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual And_condition* copy() const;
        virtual const QString description() const;
    };

class KDE_EXPORT Or_condition
    : public Condition_list_base
    {
    typedef Condition_list_base base;
    public:
        Or_condition( Condition_list_base* parent = NULL );
        Or_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P );
        virtual bool match() const;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        virtual Or_condition* copy() const;
        virtual const QString description() const;
    };


} // namespace KHotKeys

#endif // #define CONDITIONS_H
