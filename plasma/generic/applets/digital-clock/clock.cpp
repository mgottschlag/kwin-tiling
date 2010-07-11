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
#include <KSharedConfig>
#include <KColorScheme>
#include <KGlobalSettings>
#include <KConfigDialog>
#include <KCalendarSystem>
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
      m_dateTimezoneBesides(false),
      m_layout(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    // this catalog is only used once on the first start of the clock to translate the timezone in the configuration file
    KGlobal::locale()->insertCatalog("timezones4");
    setHasConfigurationInterface(true);
    resize(150, 75);
}

Clock::~Clock()
{
}

void Clock::init()
{
    ClockApplet::init();
    
    configChanged();

    dataEngine("time")->connectSource(currentTimezone(), this, updateInterval(), intervalAlignment());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), SLOT(resetSize()));
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    ClockApplet::constraintsEvent(constraints);

    if (constraints & Plasma::SizeConstraint) {
        updateSize();
    }
}

void Clock::resetSize()
{
    // Called when the size of the applet may change externally, such as on
    // font size changes
    constraintsEvent(Plasma::SizeConstraint);
}

void Clock::updateSize()
{
    Plasma::FormFactor f = formFactor();

    if (f != Plasma::Vertical && f != Plasma::Horizontal) {
        const QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
        // calculates based on size of "23:59"!
        const QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
        setMinimumSize(metrics.size(Qt::TextSingleLine, timeString));
    }

    // more magic numbers
    int aspect = 2;
    if (m_showSeconds) {
        aspect = 3;
    }

    int w, h;
    if (m_showDate || showTimezone()) {
        const QFont f(KGlobalSettings::smallestReadableFont());
        const QFontMetrics metrics(f);
        // if there's enough vertical space, wrap the words
        if (contentsRect().height() < f.pointSize() * 6) {
            QSize s = metrics.size(Qt::TextSingleLine, m_dateString);
            w = s.width() + metrics.width(" ");
            h = f.pointSize();
            //kDebug(96669) << "uS: singleline" << w;
        } else {
            QSize s = metrics.size(Qt::TextWordWrap, m_dateString);
            w = s.width();
            h = f.pointSize();
            //kDebug(96669) << "uS: wordwrap" << w;
        }

        if (!m_dateTimezoneBesides) {
            w = qMax(w, (int)(contentsRect().height() * aspect));
            h = h+(int)(contentsRect().width() / aspect);
        } else {
            w = w+(int)(contentsRect().height() * aspect);
            h = qMax(h, (int)(contentsRect().width() / aspect));
        }
    } else {
        w = (int)(contentsRect().height() * aspect);
        h = (int)(contentsRect().width() / aspect);
    }

    if (f == Plasma::Horizontal) {
        // We have a fixed height, set some sensible width
        setMinimumWidth(w);
        setMinimumHeight(0);
        //kDebug() << "DR" << m_dateRect.width() << "CR" << contentsRect().height() * aspect;
        // kDebug(96669) << contentsRect();
    } else {
        // We have a fixed width, set some sensible height
        setMinimumHeight(h);
        setMinimumWidth(0);
    }
    // kDebug(96669) << "minZize: " << minimumSize();
}

void Clock::clockConfigChanged()
{
    KConfigGroup cg = config();
    m_showTimezone = cg.readEntry("showTimezone", !isLocalTimezone());

    kDebug() << "showTimezone:" << m_showTimezone;

    m_showDate = cg.readEntry("showDate", false);
    m_showYear = cg.readEntry("showYear", false);
    m_showDay = cg.readEntry("showDay", true);

    m_showSeconds = cg.readEntry("showSeconds", false);
    if (m_showSeconds) {
        //We don't need to cache the applet if it update every seconds
        setCacheMode(QGraphicsItem::NoCache);
    } else {
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    m_plainClockFont = cg.readEntry("plainClockFont", m_plainClockFont);
    m_useCustomColor = cg.readEntry("useCustomColor", false);
    if (m_useCustomColor) {
        m_plainClockColor = cg.readEntry("plainClockColor", m_plainClockColor);
    } else {
        updateColors();
    }

    const QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
    const QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
    setMinimumSize(metrics.size(Qt::TextSingleLine, timeString));
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

    // avoid unnecessary repaints
    if ((m_showSeconds && m_time.second() != m_lastTimeSeen.second()) ||
        m_time.minute() != m_lastTimeSeen.minute()) {
        m_lastTimeSeen = m_time;

        if (Plasma::ToolTipManager::self()->isVisible(this)) {
            updateTipContent();
        }
        updateClockApplet();

        update();
        speakTime(m_time);
    }
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Appearance"), "view-media-visualization");

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

        if (m_showSeconds) {
            //We don't need to cache the applet if it update every seconds
            setCacheMode(QGraphicsItem::NoCache);
        } else {
            setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }

        changeEngineTimezone(currentTimezone(), currentTimezone());
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
        updateColors();
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
    m_lastTimeSeen = QTime();
    dataEngine("time")->disconnectSource(oldTimezone, this);
    dataEngine("time")->connectSource(newTimezone, this, updateInterval(), intervalAlignment());
}

QRectF Clock::normalLayout(int subtitleWidth, int subtitleHeight, const QRect &contentsRect)
{
    Q_UNUSED(subtitleWidth);

    QRectF myRect = QRectF(contentsRect.left(),
                    contentsRect.bottom() - subtitleHeight,
                    contentsRect.width(),
                    contentsRect.bottom());

    //p->fillRect(myRect, QBrush(QColor("green")));

    // Now find out how much space is left for painting the time
    m_timeRect = QRect(contentsRect.left(),
                       contentsRect.top(),
                       contentsRect.width(),
                       contentsRect.height() - subtitleHeight);

    return myRect;
}

QRectF Clock::sideBySideLayout(int subtitleWidth, int subtitleHeight, const QRect &contentsRect)
{
    QRectF myRect = QRectF(contentsRect.right()-subtitleWidth,
                           contentsRect.top() + (contentsRect.height()-subtitleHeight)/2,
                           subtitleWidth,
                           subtitleHeight);
    // kDebug(96669) << "myRect: " << myRect;
    // p->fillRect(myRect, QBrush(QColor("grey")));

    // Now find out how much space is left for painting the time
    m_timeRect = QRect(contentsRect.left(),
                       contentsRect.top(),
                       contentsRect.right() - subtitleWidth,
                       contentsRect.bottom());

    return myRect;
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option);

    if (!m_time.isValid() || !m_date.isValid()) {
        return;
    }

    p->setPen(QPen(m_plainClockColor));
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    p->setRenderHint(QPainter::Antialiasing);

    /* ... helps debugging contentsRect and sizing ...
       QColor c = QColor(Qt::blue);
       c.setAlphaF(.5);
       p->setBrush(c);
       p->drawRect(contentsRect);
     */

    // Paint the date, conditionally, and let us know afterwards how much
    // space is left for painting the time on top of it.
    QRectF dateRect;
    const QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);
    const QString fakeTimeString = KGlobal::locale()->formatTime(QTime(23,59,59), m_showSeconds);
    QFont smallFont = KGlobalSettings::smallestReadableFont();

    //create the string for the date and/or the timezone
    if (m_showDate || showTimezone()) {
        QString dateString;

        //Create the localized date string if needed
        if (m_showDate) {
            // JPL This needs a complete rewrite for l10n issues
            KLocale tmpLocale(*KGlobal::locale());
            tmpLocale.setCalendar(calendar()->calendarType()); 
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

        dateString = dateString.trimmed();

        if (m_dateString != dateString) {
            // If this string has changed (for example due to changes in the config
            // we have to reset the sizing of the applet
            m_dateString = dateString;
            updateSize();
        }

        // Check sizes
        // magic 10 is for very big spaces,
        // where there's enough space to grow without harming time space
        QFontMetrics fm(smallFont);

        if (contentsRect.height() > contentsRect.width() * 2) {
            //kDebug() << Plasma::Vertical << contentsRect.height() <<contentsRect.width() * 2;
            QRect dateRect = contentsRect;
            dateRect.setHeight(dateRect.width());
            smallFont.setPixelSize(qMax(dateRect.height() / 2, fm.ascent()));
            m_dateRect = preparePainter(p, dateRect, smallFont, dateString);
        } else {
            // Find a suitable size for the date font
            if (formFactor() == Plasma::Vertical) {
                smallFont.setPixelSize(qMax(contentsRect.height()/6, fm.ascent()));
            } else if (formFactor() == Plasma::Horizontal) {
                smallFont.setPixelSize(qMax(qMin(contentsRect.height(), contentsRect.width())*2/7, fm.ascent()));

                //we want to write the date always on one line
                fm = QFontMetrics(smallFont);
                const int tempWidth = fm.width(dateString);
                if(tempWidth > contentsRect.width()){
                    smallFont.setPixelSize((contentsRect.width() * smallFont.pixelSize())/tempWidth);
                }

            } else {
                smallFont.setPixelSize(qMax(qMin(contentsRect.height(), contentsRect.width())/8, KGlobalSettings::smallestReadableFont().pointSize()));
            }

            m_dateRect = preparePainter(p, contentsRect, smallFont, dateString);
        }

        // kDebug(96669) << "m_dateRect: " << m_dateRect;

        const int subtitleHeight = m_dateRect.height();
        const int subtitleWidth = m_dateRect.width();
        // kDebug(96669) << "subtitleWitdh: " << subtitleWitdh;
        // kDebug(96669) << "subtitleHeight: " << subtitleHeight;

        if (m_dateTimezoneBesides) {
            //kDebug(96669) << contentsRect.height() << subtitleHeight << smallFont.pixelSize();
            if (contentsRect.height() - subtitleHeight >= smallFont.pixelSize() || formFactor() != Plasma::Horizontal) {
                // to small to display the time on top of the date/timezone
                // put them side by side
                // kDebug(96669) << "switching to normal";
                m_dateTimezoneBesides = false;
                dateRect = normalLayout(subtitleWidth, subtitleHeight, contentsRect);
            } else {
                dateRect = sideBySideLayout(subtitleWidth, subtitleHeight, contentsRect);
            }
        } else {
            /* kDebug(96669) << "checking timezone placement"
                          << contentsRect.height() << dateRect.height() << subtitleHeight
                          << smallFont.pixelSize() << smallFont.pointSize();*/
            if (contentsRect.height() - subtitleHeight < smallFont.pixelSize() && formFactor() == Plasma::Horizontal) {
                // to small to display the time on top of the date/timezone
                // put them side by side
                // kDebug(96669) << "switching to s-b-s";
                m_dateTimezoneBesides = true;
                dateRect = sideBySideLayout(subtitleWidth, subtitleHeight, contentsRect);
            } else {
                dateRect = normalLayout(subtitleWidth, subtitleHeight, contentsRect);
            }
        }
    } else {
        m_timeRect = contentsRect;
    }
    // kDebug(96669) << "timeRect: " << m_timeRect;
    // p->fillRect(timeRect, QBrush(QColor("red")));

    // kDebug(96669) << m_time;
    // Choose a relatively big font size to start with
    m_plainClockFont.setPointSizeF(qMax(m_timeRect.height(), KGlobalSettings::smallestReadableFont().pointSize()));
    preparePainter(p, m_timeRect, m_plainClockFont, fakeTimeString, true);
    
    if (!m_dateString.isEmpty()) {
        if (m_dateTimezoneBesides) {
            QFontMetrics fm(m_plainClockFont);
            //kDebug() << dateRect << m_timeRect << fm.boundingRect(m_timeRect, Qt::AlignCenter, timeString);
            QRect br = fm.boundingRect(m_timeRect, Qt::AlignCenter, timeString);

            QFontMetrics smallfm(smallFont);
            dateRect.moveLeft(br.right() + qMin(0, br.left()) + smallfm.width(" "));
        }

        // When we're relatively low, force everything into a single line
        QFont f = p->font();
        p->setFont(smallFont);

        if (formFactor() == Plasma::Horizontal && (contentsRect.height() < smallFont.pointSize()*6)) {
            p->drawText(dateRect, Qt::TextSingleLine | Qt::AlignHCenter, m_dateString);
        } else {
            p->drawText(dateRect, Qt::TextWordWrap | Qt::AlignHCenter, m_dateString);
        }

        p->setFont(f);
    }
    
    QFontMetrics fm(p->font());
    
    p->drawText(QPoint(m_timeRect.center().x() - fm.width(fakeTimeString) / 2,
                       m_timeRect.center().y() + fm.height() / 3), timeString);
}

QRect Clock::preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, bool singleline)
{
    QRect tmpRect;
    QFont tmpFont = font;
    bool first = true;

    // Starting with the given font, decrease its size until it'll fit in the
    // given rect allowing wrapping where possible
    do {
        if (first) {
            first = false;
        } else  {
            tmpFont.setPointSize(qMax(KGlobalSettings::smallestReadableFont().pointSize(), tmpFont.pointSize() - 1));
        }

        const QFontMetrics fm(tmpFont);
        int flags = (singleline || ((formFactor() == Plasma::Horizontal) &&
                                    (contentsRect().height() < tmpFont.pointSize()*6))) ?
                    Qt::TextSingleLine : Qt::TextWordWrap;

        tmpRect = fm.boundingRect(rect, flags, text);
    } while (tmpFont.pointSize() > KGlobalSettings::smallestReadableFont().pointSize() &&
             (tmpRect.width() > rect.width() || tmpRect.height() > rect.height()));

    p->setFont(tmpFont);
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
        m_plainClockColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
        update();
    }
}
#include "clock.moc"
