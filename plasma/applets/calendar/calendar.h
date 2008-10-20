/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
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

#ifndef CALENDARTEST_H
#define CALENDARTEST_H

#include <plasma/applet.h>
#include <plasma/widgets/label.h>

#include "libplasmaclock/calendar.h"

class CalendarTest : public Plasma::Applet
{
    Q_OBJECT
    public:
        CalendarTest(QObject *parent, const QVariantList &args);
        ~CalendarTest();

        void init();
        Qt::Orientations expandingDirections() const;
        QSizeF contentSizeHint() const;
        void constraintsEvent(Plasma::Constraints constraints);

    protected slots:
        void configAccepted();

    private:
        bool m_sizedirty;
        Plasma::Calendar *cwdg;
};

K_EXPORT_PLASMA_APPLET(calendar, CalendarTest)

#endif
