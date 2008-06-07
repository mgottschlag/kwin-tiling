/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2008 by Sebastian Kuegler <sebas@kde.org>          *
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
#include <QtCore/QDate>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KConfigDialog>
#include <KDatePicker>
#include <plasma/theme.h>
#include <plasma/dialog.h>


Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_plainClockFont(KGlobalSettings::generalFont()),
      m_useCustomColor(false),
      m_plainClockColor(),
      m_showDate(false),
      m_showYear(false),
      m_showDay(false),
      m_showSeconds(false),
      m_showTimezone(false),
      m_timeZones(),
      m_calendar(0),
      m_layout(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    setHasConfigurationInterface(true);
    resize(150, 75);
}

void Clock::init()
{
    KConfigGroup cg = config();
    m_localTimeZone = cg.readEntry("localTimeZone", true);
    m_timezone = cg.readEntry("timezone", "Local");
    m_timeZones = cg.readEntry("timeZones", QStringList());

    m_showTimezone = cg.readEntry("showTimezone", (m_timezone != "Local"));

    kDebug() << "showTimezone:" << m_showTimezone;

    m_showDate = cg.readEntry("showDate", false);
    m_showYear = cg.readEntry("showYear", false);
    m_showDay = cg.readEntry("showDay", true);

    m_showSeconds = cg.readEntry("showSeconds", false);
    m_plainClockFont = cg.readEntry("plainClockFont", m_plainClockFont);
    m_useCustomColor = cg.readEntry("useCustomColor", false);
    if (m_useCustomColor) {
        m_plainClockColor = cg.readEntry("plainClockColor", m_plainClockColor);
    } else {
        m_plainClockColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    }

    QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
    QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
    setMinimumSize(metrics.size(Qt::TextSingleLine, timeString));

    m_toolTipIcon = KIcon("chronometer").pixmap(IconSize(KIconLoader::Desktop));

    dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));    
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        updateSize();
    }
}

void Clock::updateSize() {
    int aspect = 2;
    if (m_showSeconds) {
        aspect = 3;
    }
    if (formFactor() == Plasma::Horizontal) {
        // We have a fixed height, set some sensible width
        setMinimumSize(contentsRect().height() * aspect, 0);
    } else if (formFactor() == Plasma::Vertical) {
        // We have a fixed width, set some sensible height
        setMinimumSize(0, (int)contentsRect().width() / aspect);
    }
}

void Clock::updateToolTipContent() {
    QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);
    //FIXME Port to future tooltip manager
    /*Plasma::ToolTipData tipData;

    tipData.mainText = m_time.toString(timeString);
    tipData.subText = m_date.toString();
    tipData.image = m_toolTipIcon;

    setToolTip(tipData);*/
}

void Clock::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();
    m_prettyTimezone = data["Timezone City"].toString();
    m_prettyTimezone.replace("_", " ");

    updateToolTipContent();

    // avoid unnecessary repaints
    if (m_showSeconds || m_time.minute() != m_lastTimeSeen.minute()) {
        m_lastTimeSeen = m_time;
        update();
    }
}

void Clock::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
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
        Plasma::DataEngine::Data data = dataEngine("time")->query(m_timezone);
        m_calendarUi.kdatepicker->setDate(data["Date"].toDate());
        m_calendar->move(popupPosition(m_calendar->sizeHint()));
        m_calendar->show();
    }
}

void Clock::createConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    parent->addPage(widget, parent->windowTitle(), icon());
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

    ui.showDate->setChecked(m_showDate);
    ui.showYear->setChecked(m_showYear);
    ui.showDay->setChecked(m_showDay);
    ui.secondsCheckbox->setChecked(m_showSeconds);
    ui.showTimezone->setChecked(m_showTimezone);
    ui.plainClockFontBold->setChecked(m_plainClockFont.bold());
    ui.plainClockFontItalic->setChecked(m_plainClockFont.italic());
    ui.plainClockFont->setCurrentFont(m_plainClockFont);
    ui.useCustomColor->setChecked(m_useCustomColor);
    ui.plainClockColor->setColor(m_plainClockColor);
    ui.localTimeZone->setChecked(m_localTimeZone);
    ui.timeZones->setEnabled(!m_localTimeZone);
    foreach (const QString &str, m_timeZones) {
        ui.timeZones->setSelected(str, true);
    }
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();

    //We need this to happen before we disconnect/reconnect sources to ensure
    //that the update interval is set properly.
    if (m_showSeconds != ui.secondsCheckbox->isChecked()) {
        m_showSeconds = !m_showSeconds;
        cg.writeEntry("showSeconds", m_showSeconds);
    }

    if (m_localTimeZone != ui.localTimeZone->isChecked()) {
        m_localTimeZone = !m_localTimeZone;
        cg.writeEntry("localTimeZone", m_localTimeZone);
    }

    m_timeZones = ui.timeZones->selection();
    cg.writeEntry("timeZones", m_timeZones);

    if (m_localTimeZone) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
        cg.writeEntry("timezone", m_timezone);
    } else if (m_timeZones.count() > 0) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        // We have changed the timezone, show that in the clock, but only if this
        // setting hasn't been changed.
        m_timezone = m_timeZones.at(0);
        cg.writeEntry("timezone", m_timezone);
        dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        dataEngine("time")->connectSource(m_timezone, this, updateInterval(), intervalAlignment());
        cg.writeEntry("timezone", m_timezone);
    } else {
        kDebug() << "User didn't use local timezone but also didn't select any other.";
    }

    m_showDate = ui.showDate->checkState() == Qt::Checked;
    cg.writeEntry("showDate", m_showDate);
    m_showYear = ui.showYear->checkState() == Qt::Checked;
    cg.writeEntry("showYear", m_showYear);
    m_showDay = ui.showDay->checkState() == Qt::Checked;
    cg.writeEntry("showDay", m_showDay);
    m_showSeconds = ui.secondsCheckbox->checkState() == Qt::Checked;
    cg.writeEntry("showSeconds", m_showSeconds);

    if (m_showTimezone != ui.showTimezone->isChecked()) {
        m_showTimezone = !m_showTimezone;
        cg.writeEntry("showTimezone", m_showTimezone);
        kDebug() << "Saving show timezone: " << m_showTimezone;
    }

    m_plainClockFont = ui.plainClockFont->currentFont();
    m_useCustomColor = ui.useCustomColor->checkState() == Qt::Checked;
    if (m_useCustomColor) {
        m_plainClockColor = ui.plainClockColor->color();
    } else {
        m_plainClockColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    }

    m_plainClockFont.setBold(ui.plainClockFontBold->checkState() == Qt::Checked);
    m_plainClockFont.setItalic(ui.plainClockFontItalic->checkState() == Qt::Checked);

    cg.writeEntry("plainClockFont", m_plainClockFont);
    cg.writeEntry("useCustomColor", m_useCustomColor);
    cg.writeEntry("plainClockColor", m_plainClockColor);

    updateSize();
    constraintsEvent(Plasma::SizeConstraint);
    update();
    emit configNeedsSaving();
}

Clock::~Clock()
{
    delete m_calendar;
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    if (m_time.isValid() && m_date.isValid()) {
        p->setPen(QPen(m_plainClockColor));
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        p->setRenderHint(QPainter::Antialiasing);

        /* ... helps debugging contentsRect and sizing ...
        QColor c = QColor(Qt::green);
        c.setAlphaF(.5);
        p->setBrush(c);
        p->drawRect(contentsRect);
        */
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
                } else {
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
                    dateString = i18nc("@label Date with timezone: "
                                       "%1 day of the week with date, %2 timezone",
                                       "%1 %2", dateString, timezone);
                }
            } else if (m_showTimezone) {
                dateString = m_prettyTimezone;
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
                                contentsRect.width(),
                                (contentsRect.height()-subtitleHeight+4));
        } else {
            timeRect = contentsRect;
        }

        QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);

        // Choose a relatively big font size to start with
        m_plainClockFont.setPointSizeF(qMax(timeRect.height(), KGlobalSettings::smallestReadableFont().pointSize()));
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

void Clock::updateColors()
{
    if (!m_useCustomColor) {
        m_plainClockColor = KColorScheme(QPalette::Active, KColorScheme::View, Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
        update();
    }
}
#include "clock.moc"
