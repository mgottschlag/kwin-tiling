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

#include <Plasma/PopupApplet>
#include <Plasma/Label>

#include "libplasmaclock/calendar.h"

class CalendarTest : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        CalendarTest(QObject *parent, const QVariantList &args);
        ~CalendarTest();

        void init();

        /**
         * The widget that displays the calendar.
         */
        QGraphicsWidget *graphicsWidget();
    protected slots:
        void configAccepted();

    private:
        Plasma::Calendar *m_calendarDialog;
};

K_EXPORT_PLASMA_APPLET(calendar, CalendarTest)

#endif
