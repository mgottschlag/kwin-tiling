#ifndef EXISTING_WINDOW_CONDITION_H
#define EXISTING_WINDOW_CONDITION_H
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
#include <QtCore/QString>
#include <QtGui/qwindowdefs.h>

#include <KDE/KConfig>      // Needed because of some Qt Status redefinitions
#include <KDE/KConfigGroup>      // Needed because of some Qt Status redefinitions

#include <kdemacros.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#else
#define None 0
#endif

namespace KHotKeys {

class Condition_list_base;
class Windowdef_list;


/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
class KDE_EXPORT Existing_window_condition
    : public QObject, public Condition
    {
    Q_OBJECT
    typedef Condition base;
    public:
        Existing_window_condition( Windowdef_list* window_P, Condition_list_base* parent = NULL );
        Existing_window_condition( KConfigGroup& cfg_P, Condition_list_base* parent_P );
        virtual ~Existing_window_condition();
        virtual bool match() const;
        virtual void cfg_write( KConfigGroup& cfg_P ) const;
        const Windowdef_list* window() const;
        Windowdef_list* window();
        virtual Existing_window_condition* copy() const;
        virtual const QString description() const;
    public Q_SLOTS:
        void window_added( WId w_P );
        void window_removed( WId w_P );
    private:
        void init();
        void set_match( WId w_P = None );
        Windowdef_list* _window;
        bool is_match;
    };


} // namespace KHotKeys

#endif /* #ifndef EXISTING_WINDOW_CONDITION_H */
