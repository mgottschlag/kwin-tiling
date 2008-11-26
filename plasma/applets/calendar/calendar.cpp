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

#include <QGraphicsLayout>
#include <QPainter>

#include <KDebug>
#include <KIcon>

#include <Plasma/Svg>
#include <Plasma/Theme>

#include "calendar.h"


CalendarTest::CalendarTest(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
    m_calendarDialog(0),
    m_theme(0)
{
    resize(330, 240);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
}

void CalendarTest::init()
{
    setPopupIcon("view-pim-calendar");
}

QGraphicsWidget *CalendarTest::graphicsWidget()
{
    if (!m_calendarDialog) {
        m_calendarDialog = new Plasma::Calendar(this);
        m_calendarDialog->setMinimumSize(330, 240);
        m_calendarDialog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    return m_calendarDialog;
}

CalendarTest::~CalendarTest()
{

}

void CalendarTest::constraintsEvent(Plasma::Constraints constraints)
{
    if (!m_calendarDialog) {
        graphicsWidget();
    }

    if ((constraints|Plasma::FormFactorConstraint || constraints|Plasma::SizeConstraint) && 
        layout()->itemAt(0) != m_calendarDialog) {
        paintIcon();
    }
}

void CalendarTest::paintIcon()
{
    //TODO: connect to a dataengine to repaint this thing on date change
    const int iconSize = qMin(size().width(), size().height());
    QPixmap icon(iconSize, iconSize);

    if (!m_theme) {
        m_theme = new Plasma::Svg(this);
        m_theme->setImagePath("calendar/mini-calendar");
        m_theme->setContainsMultipleImages(true);
    }

    icon.fill(Qt::transparent);
    QPainter p(&icon);

    m_theme->paint(&p, icon.rect(), "mini-calendar");
    m_theme->paint(&p, icon.rect(), QString::number(QDate::currentDate().day()));
    m_theme->resize();
    p.end();
    setPopupIcon(icon);
}

void CalendarTest::configAccepted()
{
    update();
}

#include "calendar.moc"
