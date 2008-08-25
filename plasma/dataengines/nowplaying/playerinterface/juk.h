/*
 *   Copyright 2007 Alex Merry <alex.merry@kdemail.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef JUK_H
#define JUK_H

#include "playerfactory.h"

class JukFactory : public DBusPlayerFactory
{
    Q_OBJECT

public:
    explicit JukFactory(QObject* parent = 0);
    Player::Ptr create(const QVariantList& args = QVariantList());
    bool matches(const QString& serviceName);
};


#endif // JUK_H
