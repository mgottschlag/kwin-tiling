/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick                          *
 *   siraj@kdemail.net                                                     *
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

#ifndef CLOCK_H
#define CLOCK_H

#include <QtCore/QTime>
#include <QtCore/QDate>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include "ui_calendar.h"
#include <KDatePicker>


class KDialog;

namespace Plasma
{
    class Svg;
}

class ClockApplet : public Plasma::Applet
{
    Q_OBJECT
    public:
        ClockApplet(QObject *parent, const QVariantList &args);
        ~ClockApplet();

        void mousePressEvent(QGraphicsSceneMouseEvent *event);

    protected slots:
        void showCalendar(QGraphicsSceneMouseEvent *event);

    private:
        void updateToolTipContent();

        Plasma::Dialog *m_calendar;
        Ui::calendar m_calendarUi;
        QVBoxLayout *m_layout;
};

#endif
