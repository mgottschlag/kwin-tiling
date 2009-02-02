#ifndef ACTIVE_WINDOW_CONDITION_H
#define ACTIVE_WINDOW_CONDITION_H
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

#include <QtCore/QObject>

#include <QtGui/qwindowdefs.h>


class KConfigGroup;

namespace KHotKeys {

class Condition_list_base;
class Windowdef_list;

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT Active_window_condition
    : public QObject, public Condition
    {
    Q_OBJECT
    typedef Condition base;
    public:
        Active_window_condition(Windowdef_list* window_P, Condition_list_base* parent_P = NULL);
        Active_window_condition(KConfigGroup& cfg_P, Condition_list_base* parent_P);
        virtual ~Active_window_condition();
        virtual bool match() const;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        const Windowdef_list* window() const;
        Windowdef_list* window();
        virtual Active_window_condition* copy() const;
        virtual const QString description() const;
    public Q_SLOTS:
        void active_window_changed( WId );
    private:
        void init();
        void set_match();
        Windowdef_list* _window;
        bool is_match;
    };

} // namespace KHotKeys

#endif /* #ifndef ACTIVE_WINDOW_CONDITION_H */
