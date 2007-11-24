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

#include "clock.h"
#include "clocknumber.h"

#include <math.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSpinBox>
#include <QTimeLine>
#include <KGlobalSettings>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>

#include <plasma/svg.h>

#include <clocknumber.h>


Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_clockStyle(0),
      m_plainClockFontBold(0),
      m_plainClockFontItalic(0),
      m_showDate(0),
      m_showYear(0),
      m_showDay(0),
      m_showTimezone(0),
      m_dialog(0)
{
    setHasConfigurationInterface(true);
}

void Clock::init()
{
    KConfigGroup cg = config();
    m_timezone = cg.readEntry("timezone", "Local");
    if (m_timezone != "Local") {
        m_showTimezone = true;
    }

    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        m_showDate = cg.readEntry("showDate", false);
        m_showYear = cg.readEntry("showYear", false);
    } else {
        m_showDate = cg.readEntry("showDate", false);
        m_showYear = cg.readEntry("showYear", false);
    }
    m_showDay = cg.readEntry("showDay", true);

    // Default to show the timezone if it's not "Local"
    //m_showTimezone = cg.readEntry("showTimezone", m_showTimezone);
    m_showTimezone = false; // FIXME: Remove

    m_clockStyle = PlainClock;
    m_plainClockFont = cg.readEntry("plainClockFont", KGlobalSettings::generalFont());

    // FIXME: Plasma Colorscheme?, FIXME: Saving QColor to KConfig doesn't work
    //m_plainClockColor = cg.readEntry("plainClockColor", Qt::white);
    m_plainClockColor = Qt::white;
    m_plainClockFontBold = cg.readEntry("plainClockFontBold", true);
    m_plainClockFontItalic = cg.readEntry("plainClockFontItalic", false);
    m_plainClockFont.setBold(m_plainClockFontBold);
    m_plainClockFont.setItalic(m_plainClockFontItalic);

    // Set default spacings
    m_horizontalSpacing = 2;
    m_verticalSpacing = 1;

    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
}

Qt::Orientations Clock::expandingDirections() const
{
    return Qt::Vertical;
}

QSizeF Clock::contentSizeHint() const
{
    return m_sizeHint;
}

void Clock::constraintsUpdated(Plasma::Constraints)
{
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        m_sizeHint = QSize(200, 72);
    } else {
        m_sizeHint = QSize(120, 48);
    }
    updateGeometry();
}

void Clock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();

    if (m_time.minute() == m_lastTimeSeen.minute()) {
        // avoid unnecessary repaints
        return;
    }

    m_lastTimeSeen = m_time;

    update();
}

void Clock::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Clock") );

        ui.setupUi(m_dialog->mainWidget());
        ui.showDate->setChecked(m_showDate);
        ui.showYear->setChecked(m_showYear);
        ui.showDay->setChecked(m_showDay);
        ui.showTimezone->setChecked(m_showTimezone);
        ui.plainClockFontBold->setChecked(m_plainClockFontBold);
        ui.plainClockFontItalic->setChecked(m_plainClockFontItalic);
        ui.plainClockFont->setCurrentFont(m_plainClockFont);
        ui.plainClockColor->setColor(m_plainClockColor);
        ui.timeZones->setSelected(m_timezone, true);

        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }

    ui.timeZones->setSelected(m_timezone, true);
    m_dialog->show();
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();
    //QGraphicsItem::update();
    QStringList tzs = ui.timeZones->selection();

    if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            // We have changed the timezone, show that in the clock, but only if this
            // setting hasn't been changed.
            ui.showTimezone->setCheckState(Qt::Checked);
            m_timezone = tz;
            dataEngine("time")->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
        }
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        dataEngine("time")->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
    } else {
        kDebug() << "Timezone unknown: " << tzs;
    }

    m_showDate = ui.showDate->checkState() == Qt::Checked;
    cg.writeEntry("showDate", m_showDate);
    m_showYear = ui.showYear->checkState() == Qt::Checked;
    cg.writeEntry("showYear", m_showYear);
    m_showDay = ui.showDay->checkState() == Qt::Checked;
    cg.writeEntry("showDay", m_showDay);

    if (m_showTimezone != (ui.showTimezone->checkState() == Qt::Checked)) {
        cg.writeEntry("showTimezone", m_showTimezone);
    }
    m_showTimezone = ui.showTimezone->checkState() == Qt::Checked;

    m_plainClockFont = ui.plainClockFont->currentFont();
    m_plainClockColor = ui.plainClockColor->color();
    m_plainClockFontBold = ui.plainClockFontBold->checkState() == Qt::Checked;
    m_plainClockFontItalic = ui.plainClockFontItalic->checkState() == Qt::Checked;

    m_plainClockFont.setBold(m_plainClockFontBold);
    m_plainClockFont.setItalic(m_plainClockFontItalic);

    cg.writeEntry("PlainClock", m_clockStyle == PlainClock);
    cg.writeEntry("PlainClockFont", m_plainClockFont);
    cg.writeEntry("PlainClockColor", m_plainClockColor);
    cg.writeEntry("PlainClockFontBold", m_plainClockFontBold);
    cg.writeEntry("PlainClockFontItalic", m_plainClockFontItalic);

    update();
    cg.config()->sync();
}

Clock::~Clock()
{
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);
    //TODO Draw grey rectangle around numbers, as in Nuno's idea and in his mock. Should be in the svg.

    if ( !m_time.isValid() || !m_date.isValid() ) {
        return;
    }

    p->setFont(KGlobalSettings::smallestReadableFont());

    const QString hours = m_time.toString("HH");
    const QString minutes = m_time.toString("mm");
    const QString day = m_date.toString("dd");
    const QString month = m_date.toString("MMM");
    const QString year = m_date.toString("yyyy");

    p->setPen(QPen(m_plainClockColor));
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    QRect timeRect;

    // Paint the date, conditionally, and let us know afterwards how much
    // space is left for painting the time on top of it.
    if (m_showDate || m_showTimezone) {
        QString dateString;
        if (m_showDate) {
            dateString = day + ' ' + month + ' ';
            if (m_showDay) {
                QString weekday = QDate::longDayName(m_date.dayOfWeek()); // FIXME: Respect timezone settings
                dateString = weekday + ", "  + dateString;
            }
            if (m_showYear) {
                dateString += year + ' ';
            }
        }
        if (m_showTimezone) {
            dateString += m_timezone;
            dateString.replace("_", " ");
        }

        // Check sizes
        p->setFont(KGlobalSettings::smallestReadableFont());
        QRect dateRect = p->boundingRect(contentsRect,
                    QPainter::TextAntialiasing,
                    dateString);
        int subtitleHeight = dateRect.height();
        if (dateRect.width() > contentsRect.width()) {
            subtitleHeight = dateRect.height()*2;
        }

        p->drawText( QRectF(0,
                            contentsRect.bottom()-subtitleHeight,
                            contentsRect.right(),
                            contentsRect.bottom()) ,
                    dateString,
                    QTextOption(Qt::AlignHCenter)
                );

        // Now find out how much space is left for painting the time
        timeRect = QRect(   contentsRect.left(),
                            contentsRect.top(),
                            (int)(contentsRect.width()),
                            (int)(contentsRect.height()-subtitleHeight));
    } else {
        timeRect = QRect(   contentsRect.left(),
                            (int)(contentsRect.top()),
                            (int)(contentsRect.width()),
                            (int)(contentsRect.height()));
    }

    QString timeString = hours + ":" + minutes;

    m_plainClockFont.setBold(m_plainClockFontBold);
    m_plainClockFont.setItalic(m_plainClockFontItalic);

    // Choose a relatively big font size to start with and decrease it from there to fit.
    m_plainClockFont.setPointSize(qMax((int)(contentsRect.height()/1.5), 1));
    p->setFont(m_plainClockFont);
    QRect tmpTimeRect = p->boundingRect(timeRect, QPainter::TextAntialiasing, timeString);

    while ((tmpTimeRect.width() > timeRect.width() || tmpTimeRect.height() > timeRect.height()) && m_plainClockFont.pointSize() > 1) {
	m_plainClockFont.setPointSize(m_plainClockFont.pointSize() - 1);
	p->setFont(m_plainClockFont);
	tmpTimeRect = p->boundingRect(timeRect, QPainter::TextAntialiasing, timeString);
    }

    p->drawText(timeRect,
		timeString,
		QTextOption(Qt::AlignHCenter)
	    );
    return;
}

#include "clock.moc"
