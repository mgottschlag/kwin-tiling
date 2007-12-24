/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef LOCKOUT_H
#define LOCKOUT_H

#include <plasma/applet.h>


class LockOut : public Plasma::Applet
{
    Q_OBJECT

    public:
        LockOut(QObject *parent, const QVariantList &args);
        ~LockOut();

        QSizeF contentSizeHint() const;
        Qt::Orientations expandingDirections() const;

    public slots:
        void clickLogout();
        void clickLock();
};

K_EXPORT_PLASMA_APPLET(lockout, LockOut)

#endif
