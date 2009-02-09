#ifndef WINDOW_SELECTION_LIST_H
#define WINDOW_SELECTION_LIST_H
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

#include "windows_helper/window_selection_interface.h"

#include <QtCore/QList>

namespace KHotKeys {


class KDE_EXPORT Windowdef_list : public QList< Windowdef* >
    {
    Q_DISABLE_COPY( Windowdef_list )

    public:
        Windowdef_list( const QString& comment = "" );
        Windowdef_list( KConfigGroup& cfg_P/*, ActionDataBase* data_P*/ );

        ~Windowdef_list();

        void cfg_write( KConfigGroup& cfg_P ) const;
        bool match( const Window_data& window_P ) const;
        Windowdef_list* copy( /*ActionDataBase* data_P*/ ) const;
        // typedef QList< Windowdef* >::iterator Iterator;
        void set_comment(const QString &comment);
        const QString& comment() const;
    private:
        QString _comment;
    };


} // namespace KHotKeys

#endif /* #ifndef WINDOW_SELECTION_LIST_H */
