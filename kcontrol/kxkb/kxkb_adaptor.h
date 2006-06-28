/* This file is part of the KDE project
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KXKB_ADAPTOR_H
#define KXKB_ADAPTOR_H

#include <dbus/qdbus.h>
#include "kxkb.h"

// Not really an adaptor, more like the object exposed to dbus
// This allows this object to have the dbus path "/kxkb" instead of "/MainApplication",
// in order to be independent from the implementation in kxkb.
class KXKBAdaptor : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KXKB")

public:
    KXKBAdaptor( KXKBApp* app );

    inline KXKBApp* parent() const { return static_cast<KXKBApp *>( QObject::parent() ); }

public Q_SLOTS: // DBus exported
    Q_SCRIPTABLE bool setLayout(const QString& layout);
    Q_SCRIPTABLE QString getCurrentLayout();
    Q_SCRIPTABLE QStringList getLayoutsList();
    Q_SCRIPTABLE void forceSetXKBMap( bool set );

};
#endif /* KXKB_ADAPTOR_H */
