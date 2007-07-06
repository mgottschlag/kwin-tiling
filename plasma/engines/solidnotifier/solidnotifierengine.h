/*
 * Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/


#ifndef SOLIDNOTIFIERENGINE_H
#define SOLIDNOTIFIERENGINE_H

#include <QObject>
#include <QString>
#include <QList>

#include "plasma/dataengine.h"

/**
 * This class evaluates the basic expressions given in the interface.
 */
class SolidNotifierEngine : public Plasma::DataEngine
{
    Q_OBJECT

    public:
        SolidNotifierEngine( QObject* parent, const QStringList& args);
        ~SolidNotifierEngine();
    protected slots :
        void onDeviceAdded(const QString &udi);
        void onDeviceRemoved(const QString &udi);
};

K_EXPORT_PLASMA_DATAENGINE(solidnotifier, SolidNotifierEngine)

#endif
