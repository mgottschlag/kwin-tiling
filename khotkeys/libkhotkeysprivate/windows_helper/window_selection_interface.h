#ifndef WINDOW_SELECTION_INTERFACE_H
#define WINDOW_SELECTION_INTERFACE_H
/* Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   Version 2 as published by the Free Software Foundation;

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include <QtCore/QString>

#include "KConfigGroup"

#include <QtGui/qwindowdefs.h>

#include <netwm_def.h>

namespace KHotKeys {

struct KDE_EXPORT Window_data
    {
    Window_data( WId id_P );
    QString title; // _NET_WM_NAME or WM_NAME
    QString role; // WM_WINDOW_ROLE
    QString wclass; // WM_CLASS
    NET::WindowType type;
    };

class KDE_EXPORT Windowdef
    {
    Q_DISABLE_COPY( Windowdef )

    public:
        Windowdef( const QString& comment_P );
        Windowdef( KConfigGroup& cfg_P );
        virtual ~Windowdef();
        const QString& comment() const;
        void set_comment(const QString &comment);
        virtual bool match( const Window_data& window_P ) = 0;
        static Windowdef* create_cfg_read( KConfigGroup& cfg_P/*, ActionDataBase* data_P*/ );
        virtual void cfg_write( KConfigGroup& cfg_P ) const = 0;
        virtual Windowdef* copy( /*ActionDataBase* data_P*/ ) const = 0;
        virtual const QString description() const = 0;
    private:
        QString _comment;
    };

} // namespace KHotKeys

#endif /* #ifndef WINDOW_SELECTION_INTERFACE_H */
