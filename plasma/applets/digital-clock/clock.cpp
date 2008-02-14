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

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsSceneMouseEvent>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KDatePicker>
#include <plasma/theme.h>
#include <plasma/dialog.h>


Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_clockStyle(PlainClock),
      m_plainClockFont(KGlobalSettings::generalFont()),
      m_plainClockColor(),
      m_plainClockFontBold(false),
      m_plainClockFontItalic(false),
      m_showDate(false),
      m_showYear(false),
      m_showDay(false),
      m_showSeconds(false),
      m_showTimezone(false),
      m_dialog(0),
      m_calendar(0),
      m_layout(0)
{
    setHasConfigurationInterface(true);
    setContentSize(70, 22);
}

void Clock::init()
{
    KConfigGroup cg = config();
    m_timezone = cg.readEntry("timezone", "Local");

    m_showTimezone = cg.readEntry("showTimezone", (m_timezone != "Local"));

    kDebug() << "showTimezone:" << m_showTimezone;

    m_showDate = cg.readEntry("showDate", false);
    m_showYear = cg.readEntry("showYear", false);

    m_showDay = cg.readEntry("showDay", true);

    m_showSeconds = cg.readEntry("showSeconds", false);
    m_plainClockColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::self()->colors()).foreground().color();
    m_plainClockFont = cg.readEntry("plainClockFont", m_plainClockFont);
    m_plainClockColor = cg.readEntry("plainClockColor", m_plainClockColor);

    m_plainClockFontBold = cg.readEntry("plainClockFontBold", true);
    m_plainClockFontItalic = cg.readEntry("plainClockFontItalic", false);
    m_plainClockFont.setBold(m_plainClockFontBold);
    m_plainClockFont.setItalic(m_plainClockFontItalic);

    QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
    QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
    setMinimumContentSize(metrics.size(Qt::TextSingleLine, timeString));

    dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
}

Qt::Orientations Clock::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal;
    }
}

void Clock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();
    m_prettyTimezone = data["Timezone City"].toString();

    // avoid unnecessary repaints
    if (m_showSeconds || m_time.minute() != m_lastTimeSeen.minute()) {
        m_lastTimeSeen = m_time;

        update();
    }
}

void Clock::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton && contentRect().contains(event->pos())) {
        showCalendar(event);
    } else {
        event->ignore();
    }
}

void Clock::showCalendar(QGraphicsSceneMouseEvent *event)
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
        m_calendar->move(popupPosition(m_calendar->sizeHint()));
        m_calendar->show();
    }
}

void Clock::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;

        m_dialog->setCaption(i18n("Configure Clock"));

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );
    }
    ui.showDate->setChecked(m_showDate);
    ui.showYear->setChecked(m_showYear);
    ui.showDay->setChecked(m_showDay);
    ui.secondsCheckbox->setChecked(m_showSeconds);
    ui.showTimezone->setChecked(m_showTimezone);
    ui.plainClockFontBold->setChecked(m_plainClockFontBold);
    ui.plainClockFontItalic->setChecked(m_plainClockFontItalic);
    ui.plainClockFont->setCurrentFont(m_plainClockFont);
    ui.plainClockColor->setColor(m_plainClockColor);
    ui.timeZones->setSelected(m_timezone, true);
    ui.timeZones->setEnabled(m_timezone != "Local");
    ui.localTimeZone->setChecked(m_timezone == "Local");

    m_dialog->show();
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();
    //We need this to happen before we disconnect/reconnect sources to ensure
    //that the update interval is set properly.
    m_showSeconds = ui.secondsCheckbox->checkState() == Qt::Checked;
    cg.writeEntry("showSeconds", m_showSeconds);
    //QGraphicsItem::update();
    QStringList tzs = ui.timeZones->selection();
    if (ui.localTimeZone->checkState() == Qt::Checked) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
        cg.writeEntry("timezone", m_timezone);
    } else if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            // We have changed the timezone, show that in the clock, but only if this
            // setting hasn't been changed.
            ui.showTimezone->setCheckState(Qt::Checked);
            m_timezone = tz;
            dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
        }
        cg.writeEntry("timezone", m_timezone);
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
        cg.writeEntry("timezone", m_timezone);
    } else {
        kDebug() << "Timezone unknown: " << tzs;
    }

    m_showDate = ui.showDate->checkState() == Qt::Checked;
    cg.writeEntry("showDate", m_showDate);
    m_showYear = ui.showYear->checkState() == Qt::Checked;
    cg.writeEntry("showYear", m_showYear);
    m_showDay = ui.showDay->checkState() == Qt::Checked;
    cg.writeEntry("showDay", m_showDay);
    m_showSeconds = ui.secondsCheckbox->checkState() == Qt::Checked;
    cg.writeEntry("showSeconds", m_showSeconds);

    if (m_showTimezone != (ui.showTimezone->checkState() == Qt::Checked)) {
        m_showTimezone = ui.showTimezone->checkState() == Qt::Checked;
        cg.writeEntry("showTimezone", m_showTimezone);
        kDebug() << "Saving show timezone: " << m_showTimezone;
    }

    m_plainClockFont = ui.plainClockFont->currentFont();
    m_plainClockColor = ui.plainClockColor->color();
    m_plainClockFontBold = ui.plainClockFontBold->checkState() == Qt::Checked;
    m_plainClockFontItalic = ui.plainClockFontItalic->checkState() == Qt::Checked;

    m_plainClockFont.setBold(m_plainClockFontBold);
    m_plainClockFont.setItalic(m_plainClockFontItalic);

    cg.writeEntry("plainClock", m_clockStyle == PlainClock);
    cg.writeEntry("plainClockFont", m_plainClockFont);
    cg.writeEntry("plainClockColor", m_plainClockColor);
    cg.writeEntry("plainClockFontBold", m_plainClockFontBold);
    cg.writeEntry("plainClockFontItalic", m_plainClockFontItalic);

    update();
    emit configNeedsSaving();
}

Clock::~Clock()
{
    delete m_calendar;
    delete m_dialog;
    //delete m_layout;
    // deleting m_layout isn't necessary. it is owned by the dialog
    // (setLayout transfers ownership) and so will be deleted when the dialog is.
    // thnx aseigo
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    if (m_time.isValid() && m_date.isValid()) {

        p->setFont(KGlobalSettings::smallestReadableFont());

        p->setPen(QPen(m_plainClockColor));
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);

        QRect timeRect;

        // Paint the date, conditionally, and let us know afterwards how much
        // space is left for painting the time on top of it.
        if (m_showDate || m_showTimezone) {
            QString dateString;
            if (m_showDate) {
                QString day = m_date.toString("d");
                QString month = m_date.toString("MMM");
                if (m_showYear) {
                    QString year = m_date.toString("yyyy");
                    dateString = i18nc("@label Short date: "
                                       "%1 day in the month, %2 short month name, %3 year",
                                       "%1 %2 %3", day, month, year);
                }
                else {
                    dateString = i18nc("@label Short date: "
                                       "%1 day in the month, %2 short month name",
                                       "%1 %2", day, month);
                }
                if (m_showDay) {
                    QString weekday = QDate::shortDayName(m_date.dayOfWeek());
                    dateString = i18nc("@label Day of the week with date: "
                                       "%1 short day name, %2 short date",
                                       "%1, %2", weekday, dateString);
                }
                if (m_showTimezone) {
                    QString timezone = m_prettyTimezone;
                    timezone.replace("_", " ");
                    dateString = i18nc("@label Date with timezone: "
                                       "%1 day of the week with date, %2 timezone",
                                       "%1 %2", dateString, timezone);
                }
            }
            else if (m_showTimezone) {
                dateString = m_prettyTimezone;
                dateString.replace("_", " ");
            }

            // Check sizes
            QRect dateRect = preparePainter(p, contentsRect, KGlobalSettings::smallestReadableFont(), dateString);
            int subtitleHeight = dateRect.height();

            p->drawText(QRectF(0,
                                contentsRect.bottom()-subtitleHeight,
                                contentsRect.right(),
                                contentsRect.bottom()) ,
                        dateString,
                        QTextOption(Qt::AlignHCenter)
                    );

            // Now find out how much space is left for painting the time
            timeRect = QRect(   contentsRect.left(),
                                contentsRect.top(),
                                (contentsRect.width()),
                                (contentsRect.height()-subtitleHeight));
        } else {
            timeRect = QRect(   contentsRect.left(),
                                (contentsRect.top()),
                                (contentsRect.width()),
                                (contentsRect.height()));
        }
        QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);

        m_plainClockFont.setBold(m_plainClockFontBold);
        m_plainClockFont.setItalic(m_plainClockFontItalic);

        // Choose a relatively big font size to start with
        m_plainClockFont.setPointSize(qMax((int)(contentsRect.height()/1.5), KGlobalSettings::smallestReadableFont().pointSize()));
        preparePainter(p, timeRect, m_plainClockFont, timeString);

        p->drawText(timeRect,
                    timeString,
                    QTextOption(Qt::AlignCenter)
                );
    }
}

QRect Clock::preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text)
{
    QRect tmpRect;
    QFont tmpFont = font;

    // Starting with the given font, decrease its size until it'll fit in the
    // given rect allowing wrapping where possible
    do {
        p->setFont(tmpFont);
        tmpFont.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), tmpFont.pointSize() - 1));
        tmpRect = p->boundingRect(rect, Qt::TextWordWrap, text);
    } while (tmpFont.pointSize() > KGlobalSettings::smallestReadableFont().pointSize() && (tmpRect.width() > rect.width() ||
            tmpRect.height() > rect.height()));

    return tmpRect;
}

int Clock::updateInterval() const
{
    return m_showSeconds ? 1000 : 60000;
}

Plasma::IntervalAlignment Clock::intervalAlignment() const
{
    return m_showSeconds ? Plasma::NoAlignment : Plasma::AlignToMinute;
}

#include "clock.moc"
