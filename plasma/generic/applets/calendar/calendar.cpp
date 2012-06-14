/***************************************************************************
 *   Copyright 2008 by Davide Bettio <davide.bettio@kdemail.net>           *
 *   Copyright 2009 by John Layt <john@layt.net>                           *
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

#include "calendar.h"

#include <QGraphicsLayout>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <KSystemTimeZones>
#include <KConfigDialog>
#include <KConfigGroup>

#include <Plasma/Svg>
#include <Plasma/Theme>

CalendarApplet::CalendarApplet(QObject *parent, const QVariantList &args)
    : Plasma::PopupApplet(parent, args),
    m_calendarWidget(0),
    m_theme(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setCacheMode(DeviceCoordinateCache);
    m_dateUpdater = new QTimer(this);
    m_dateUpdater->setSingleShot(true);
    connect(m_dateUpdater, SIGNAL(timeout()), this, SLOT(updateDate()));
}

CalendarApplet::~CalendarApplet()
{
}

void CalendarApplet::init()
{
    setPopupIcon("view-pim-calendar");
    m_calendarWidget = new Plasma::Calendar(this);
    m_calendarWidget->setMinimumSize(220, 250);
    updateDate();
    configChanged();
    setFocusPolicy(Qt::StrongFocus);
}

void CalendarApplet::focusInEvent(QFocusEvent* event)
{
    Q_UNUSED(event);
    m_calendarWidget->setFlag(QGraphicsItem::ItemIsFocusable);
    m_calendarWidget->setFocus();
}

void CalendarApplet::configChanged()
{
    m_calendarWidget->applyConfiguration(config());
}

QGraphicsWidget *CalendarApplet::graphicsWidget()
{
    return m_calendarWidget;
}

void CalendarApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if ((constraints|Plasma::FormFactorConstraint || constraints|Plasma::SizeConstraint) &&
        layout()->itemAt(0) != m_calendarWidget) {
        paintIcon();
    }
}

void CalendarApplet::paintIcon()
{
    const int iconSize = qMin(size().width(), size().height());

    if (iconSize <= 0) {
        return;
    }

    QPixmap icon(iconSize, iconSize);

    if (!m_theme) {
        m_theme = new Plasma::Svg(this);
        m_theme->setImagePath("calendar/mini-calendar");
        m_theme->setContainsMultipleImages(true);
    }

    icon.fill(Qt::transparent);
    QPainter p(&icon);

    m_theme->paint(&p, icon.rect(), "mini-calendar");

    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    p.setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    font.setPixelSize(icon.size().height() / 2);
    p.setFont(font);
    p.drawText(icon.rect().adjusted(0, icon.size().height()/4, 0, 0), Qt::AlignCenter,
               QString::number(m_calendarWidget->calendar()->day(m_calendarWidget->date())));
    m_theme->resize();
    p.end();
    setPopupIcon(icon);
}

void CalendarApplet::configAccepted()
{
    m_calendarWidget->configAccepted(config());
    update();
}

void CalendarApplet::updateDate()
{
    QDateTime now = QDateTime::currentDateTime();
    static const int secsInDay = 24 * 60 * 60;
    const int sinceEpoch = now.toTime_t() + KSystemTimeZones::local().currentOffset();
    const int updateIn = (secsInDay) - (sinceEpoch % secsInDay);
    if (updateIn > secsInDay - 60) {
        // after midnight, we try and update right away again in case of odd clock drifting
        // that could cause us to miss (or delay) the date chagne
        m_dateUpdater->setInterval(60 * 1000);
    } else if (updateIn < m_dateUpdater->interval()) {
        m_dateUpdater->setInterval(updateIn * 1000);
    } else {
        // update once an hour
        m_dateUpdater->setInterval(60 * 60 * 1000);
    }
    //kDebug() << "updating in" << m_dateUpdater->interval();
    m_dateUpdater->start();
    paintIcon();
}

void CalendarApplet::createConfigurationInterface(KConfigDialog *parent)
{
    m_calendarWidget->createConfigurationInterface(parent);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
}

#include "calendar.moc"
