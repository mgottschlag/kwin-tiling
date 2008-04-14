/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
 *   Copyright (C) 2007 by Sebastian Kuegler <sebas@kde.org>               *
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

#include "clockapplet.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QDate>

#include <KDebug>
#include <KDialog>
#include <KColorScheme>
#include <KDatePicker>
#include <plasma/theme.h>
#include <plasma/dialog.h>


ClockApplet::ClockApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_calendar(0)
{
}

void ClockApplet::updateToolTipContent() {
    QString timeString = "";//KGlobal::locale()->formatTime(m_time, m_showSeconds);
    //TODO port to TOOLTIP manager
    /*Plasma::ToolTipData tipData;

    tipData.mainText = "";//m_time.toString(timeString);
    tipData.subText = "";//m_date.toString();
    //tipData.image = m_toolTipIcon;

    setToolTip(tipData);*/
}

void ClockApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton && geometry().contains(event->pos())) {
        showCalendar(event);
    } else {
        event->ignore();
    }
}

void ClockApplet::showCalendar(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    if (m_calendar == 0) {
        m_calendar = new Plasma::Dialog();
        //m_calendar->setStyleSheet("{ border : 0px }"); // FIXME: crashes
        m_layout = new QVBoxLayout();
        m_layout->setSpacing(0);
        m_layout->setMargin(0);

        m_calendarUi.setupUi(m_calendar);
        m_calendar->setLayout(m_layout);
        m_calendar->setWindowFlags(Qt::Popup);
        m_calendar->adjustSize();
    }

    if (m_calendar->isVisible()) {
        m_calendar->hide();
    } else {
        kDebug();
        m_calendarUi.kdatepicker->setDate(QDate::currentDate());
        m_calendar->move(popupPosition(m_calendar->sizeHint()));
        m_calendar->show();
    }
}

ClockApplet::~ClockApplet()
{
    delete m_calendar;
}

#include "clockapplet.moc"
