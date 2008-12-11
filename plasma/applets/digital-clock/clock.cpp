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
#include <Plasma/Theme>
#include <Plasma/Dialog>
#include <Plasma/ToolTipManager>


Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_plainClockFont(KGlobalSettings::generalFont()),
      m_useCustomColor(false),
      m_plainClockColor(),
      m_showDate(false),
      m_showYear(false),
      m_showDay(false),
      m_showSeconds(false),
      m_showTimezone(false),
      m_dateString(0),
      m_layout(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    setHasConfigurationInterface(true);
    resize(150, 75);
}

Clock::~Clock()
{
}

void Clock::init()
{
    ClockApplet::init();

    KConfigGroup cg = config();

    m_showTimezone = cg.readEntry("showTimezone", !isLocalTimezone());

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

    dataEngine("time")->connectSource(currentTimezone(), this, updateInterval(), intervalAlignment());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    ClockApplet::constraintsEvent(constraints);

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
        if (m_showDate || showTimezone()) {
            QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
            int w;
            if (contentsRect().height() < KGlobalSettings::smallestReadableFont().pointSize()*6) {
                w = (int)metrics.size(Qt::TextSingleLine, m_dateString).width();
            } else {
                w = (int)metrics.size(Qt::TextWordWrap, m_dateString).width();
            }
            setMinimumWidth(qMax(w, (int)(contentsRect().height() * aspect)));
        } else {
            setMinimumWidth((int)(contentsRect().height() * aspect));
        }
        setMinimumHeight(0);
        //kDebug() << "DR" << m_dateRect.width() << "CR" << contentsRect().height() * aspect;
    } else if (formFactor() == Plasma::Vertical) {
        // We have a fixed width, set some sensible height
        setMinimumHeight((int)contentsRect().width() / aspect);
        setMinimumWidth(0);
    } else {
        QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
        QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
        setMinimumSize(metrics.size(Qt::TextSingleLine, timeString));
    }
}

bool Clock::showTimezone() const
{
    return m_showTimezone || shouldDisplayTimezone();
}

void Clock::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        updateTipContent();
    }

    // avoid unnecessary repaints
    if ((m_showSeconds && m_time.second() != m_lastTimeSeen.second()) ||
        m_time.minute() != m_lastTimeSeen.minute()) {
        m_lastTimeSeen = m_time;
        update();
    }
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());

    ui.showDate->setChecked(m_showDate);
    ui.showYear->setChecked(m_showYear);
    ui.showDay->setChecked(m_showDay);
    ui.secondsCheckbox->setChecked(m_showSeconds);
    ui.showTimeZone->setChecked(m_showTimezone);
    ui.plainClockFontBold->setChecked(m_plainClockFont.bold());
    ui.plainClockFontItalic->setChecked(m_plainClockFont.italic());
    ui.plainClockFont->setCurrentFont(m_plainClockFont);
    ui.useCustomColor->setChecked(m_useCustomColor);
    ui.plainClockColor->setColor(m_plainClockColor);
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();

    m_showTimezone = ui.showTimeZone->isChecked();
    cg.writeEntry("showTimezone", m_showTimezone);

    m_plainClockFont = ui.plainClockFont->currentFont();
    //We need this to happen before we disconnect/reconnect sources to ensure
    //that the update interval is set properly.
    if (m_showSeconds != ui.secondsCheckbox->isChecked()) {
        m_showSeconds = !m_showSeconds;
        cg.writeEntry("showSeconds", m_showSeconds);

        dataEngine("time")->disconnectSource(currentTimezone(), this);
        dataEngine("time")->connectSource(currentTimezone(), this, updateInterval(), intervalAlignment());
    }

    m_showDate = ui.showDate->checkState() == Qt::Checked;
    cg.writeEntry("showDate", m_showDate);
    m_showYear = ui.showYear->checkState() == Qt::Checked;
    cg.writeEntry("showYear", m_showYear);
    m_showDay = ui.showDay->checkState() == Qt::Checked;
    cg.writeEntry("showDay", m_showDay);
    m_showSeconds = ui.secondsCheckbox->checkState() == Qt::Checked;
    cg.writeEntry("showSeconds", m_showSeconds);

    m_useCustomColor = ui.useCustomColor->isChecked();
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

    constraintsEvent(Plasma::SizeConstraint);
    update();
    emit sizeHintChanged(Qt::PreferredSize);
    emit configNeedsSaving();
}

void Clock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    dataEngine("time")->disconnectSource(oldTimezone, this);
    dataEngine("time")->connectSource(newTimezone, this, updateInterval(), intervalAlignment());
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
        if (m_showDate || showTimezone()) {
            QString dateString;
            if (m_showDate) {
                KLocale tmpLocale(*KGlobal::locale());
                tmpLocale.setDateFormat("%e"); // day number of the month
                QString day = tmpLocale.formatDate(m_date);
                tmpLocale.setDateFormat("%b"); // short form of the month
                QString month = tmpLocale.formatDate(m_date);

                if (m_showYear) {
                    tmpLocale.setDateFormat("%Y"); // whole year
                    QString year = tmpLocale.formatDate(m_date);
                    dateString = i18nc("@label Short date: "
                                       "%1 day in the month, %2 short month name, %3 year",
                                       "%1 %2 %3", day, month, year);
                } else {
                    dateString = i18nc("@label Short date: "
                                       "%1 day in the month, %2 short month name",
                                       "%1 %2", day, month);
                }

                if (m_showDay) {
                    tmpLocale.setDateFormat("%a"); // short weekday
                    QString weekday = tmpLocale.formatDate(m_date);
                    dateString = i18nc("@label Day of the week with date: "
                                       "%1 short day name, %2 short date",
                                       "%1, %2", weekday, dateString);
                }

                if (showTimezone()) {
                    QString currentTimezone = prettyTimezone();
                    dateString = i18nc("@label Date with currentTimezone: "
                                       "%1 day of the week with date, %2 currentTimezone",
                                       "%1 %2", dateString, currentTimezone);
                }
            } else if (showTimezone()) {
                dateString = prettyTimezone();
            }
            if (m_dateString != dateString) {
                // If this string has changed (for example due to changes in the config
                // we have to reset the sizing of the applet
                m_dateString = dateString;
                updateSize();
            }
            // Check sizes
            QFont f = KGlobalSettings::smallestReadableFont();
            f.setPointSizeF(qMax(contentsRect.height()/10, f.pointSize()));

            m_dateRect = preparePainter(p, contentsRect, f, dateString);
            int subtitleHeight = m_dateRect.height();

            QRectF myRect = QRectF(contentsRect.left(),
                                contentsRect.bottom()-subtitleHeight,
                                contentsRect.width(),
                                contentsRect.bottom());

            //p->fillRect(myRect, QBrush(QColor("green")));
            // When we're relatively low, force everything into a single line
            if (formFactor() == Plasma::Horizontal && (contentsRect.height() < f.pointSize()*6)) {
                p->drawText(myRect,
                        Qt::TextSingleLine | Qt::AlignHCenter,
                        dateString.trimmed());
            } else {
                p->drawText(myRect,
                        Qt::TextWordWrap | Qt::AlignHCenter,
                        dateString.trimmed());
            }

            // Now find out how much space is left for painting the time
            timeRect = QRect(   contentsRect.left(),
                                contentsRect.top(),
                                contentsRect.width(),
                                (contentsRect.height()-subtitleHeight+4));
        } else {
            timeRect = contentsRect;
        }
        //p->fillRect(timeRect, QBrush(QColor("red")));

        QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);
        // Choose a relatively big font size to start with
        m_plainClockFont.setPointSizeF(qMax(timeRect.height(), KGlobalSettings::smallestReadableFont().pointSize()));
        preparePainter(p, timeRect, m_plainClockFont, timeString, true);

        QTextOption textOption(Qt::AlignCenter);
        textOption.setWrapMode(QTextOption::NoWrap);
        p->drawText(timeRect,
                    timeString,
                    textOption
                );
    }
}

QRect Clock::preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, const bool singleline)
{
    QRect tmpRect;
    QFont tmpFont = font;

    // Starting with the given font, decrease its size until it'll fit in the
    // given rect allowing wrapping where possible
    do {
        p->setFont(tmpFont);
        tmpFont.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), tmpFont.pointSize() - 1));
        int flags = (singleline || ((formFactor() == Plasma::Horizontal) &&
                                    (contentsRect().height() < tmpFont.pointSize()*6))) ?
                    Qt::TextSingleLine : Qt::TextWordWrap;

        tmpRect = p->boundingRect(rect, flags, text);
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
