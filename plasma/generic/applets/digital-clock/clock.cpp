/***************************************************************************
 *   Copyright (C) 2007-2008 by Riccardo Iaconelli <riccardo@kde.org>      *
 *   Copyright (C) 2007-2010 by Sebastian Kügler <sebas@kde.org>           *
 *   Copyright (C) 2011      by Teo Mrnjavac <teo@kde.org>                 *
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
#include <KServiceTypeTrader>
#include <KRun>
#include <Plasma/Theme>
#include <Plasma/Dialog>
#include <Plasma/Svg>
#include <Plasma/PaintUtils>
#include <Plasma/ToolTipManager>


Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_plainClockFont(KGlobalSettings::generalFont()),
      m_useCustomColor(false),
      m_useCustomShadowColor(false),
      m_drawShadow(true),
      m_dateStyle(0),
      m_showSeconds(false),
      m_showTimezone(false),
      m_dateTimezoneBesides(false),
      m_layout(0),
      m_svg(0)
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

    dataEngine("time")->connectSource(currentTimezone(), this, updateInterval(), intervalAlignment());
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateColors()));
    connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), SLOT(resetSize()));
    connect(KGlobalSettings::self(), SIGNAL(settingsChanged(int)), SLOT(updateClock(int)));
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    ClockApplet::constraintsEvent(constraints);

    if (constraints & Plasma::SizeConstraint || constraints & Plasma::FormFactorConstraint) {
        updateSize();
    }
}

// In case time format has changed, e.g. from 24h to 12h format.
void Clock::updateClock(int category)
{
    if (category != KGlobalSettings::SETTINGS_LOCALE) {
        return;
    }

    generatePixmap();
    update();
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
    if (m_dateStyle || showTimezone()) {
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
        setMinimumSize(QSize(w, 0));
        //kDebug() << "DR" << m_dateRect.width() << "CR" << contentsRect().height() * aspect;
        // kDebug(96669) << contentsRect();
    } else {
        // We have a fixed width, set some sensible height
        setMinimumSize(QSize(0, h));
    }

    setPreferredSize(QSize(w, h));
    emit sizeHintChanged(Qt::PreferredSize);
    //kDebug(96669) << "minZize: " << minimumSize() << preferredSize();

    if (m_isDefaultFont) {
        const QString fakeTimeString = KGlobal::locale()->formatTime(QTime(23,59,59), m_showSeconds);
        expandFontToMax(m_plainClockFont, fakeTimeString);
    }

    generatePixmap();
    update();
}

void Clock::clockConfigChanged()
{
    KConfigGroup cg = config();
    m_showTimezone = cg.readEntry("showTimezone", !isLocalTimezone());

    kDebug() << "showTimezone:" << m_showTimezone;

    if (cg.hasKey("showDate")) {    //legacy config entry as of 2011-1-4
        m_dateStyle = cg.readEntry("showDate", false) ? 2 : 0; //short date : no date
        cg.deleteEntry("showDate");
    }
    else {
        m_dateStyle = cg.readEntry("dateStyle", 0);
    }

    if (cg.hasKey("showYear")) {   //legacy config entry as of 2011-1-4
        if( m_dateStyle ) {
            m_dateStyle = cg.readEntry("showYear", false) ? 2 : 1; //short date : compact date
        }
        cg.deleteEntry("showYear");
    }

    m_showSeconds = cg.readEntry("showSeconds", false);
    if (m_showSeconds) {
        //We don't need to cache the applet if it update every seconds
        setCacheMode(QGraphicsItem::NoCache);
    } else {
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    QFont f = cg.readEntry("plainClockFont", m_plainClockFont);
    m_isDefaultFont = f == m_plainClockFont;
    m_plainClockFont = f;

    m_useCustomColor = cg.readEntry("useCustomColor", m_useCustomColor);
    m_plainClockColor = cg.readEntry("plainClockColor", m_plainClockColor);
    m_useCustomShadowColor = cg.readEntry("useCustomShadowColor", m_useCustomShadowColor);
    m_plainClockShadowColor = cg.readEntry("plainClockShadowColor", m_plainClockShadowColor);
    m_drawShadow = cg.readEntry("plainClockDrawShadow", m_drawShadow);

    updateColors();

    if (m_useCustomColor) {
        m_pixmap = QPixmap();
        delete m_svg;
        m_svg = 0;
    }

    const QFontMetricsF metrics(KGlobalSettings::smallestReadableFont());
    const QString timeString = KGlobal::locale()->formatTime(QTime(23, 59), m_showSeconds);
    setMinimumSize(metrics.size(Qt::TextSingleLine, timeString));

    if (isUserConfiguring()) {
        updateSize();
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

    // avoid unnecessary repaints
    if ((m_showSeconds && m_time.second() != lastTimeSeen().second()) ||
        m_time.minute() != lastTimeSeen().minute()) {

        if (Plasma::ToolTipManager::self()->isVisible(this)) {
            updateTipContent();
        }

        updateClockApplet(data);
        generatePixmap();
        update();
    }
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Appearance"), "view-media-visualization");

    ui.secondsCheckbox->setChecked(m_showSeconds);
    ui.showTimeZone->setChecked(m_showTimezone);
    ui.plainClockFontBold->setChecked(m_plainClockFont.bold());
    ui.plainClockFontItalic->setChecked(m_plainClockFont.italic());
    ui.plainClockFont->setCurrentFont(m_plainClockFont);
    ui.useCustomColor->setChecked(m_useCustomColor);
    ui.plainClockColor->setColor(m_plainClockColor);
    ui.drawShadow->setChecked(m_drawShadow);
    ui.useCustomShadowColor->setChecked(m_useCustomShadowColor);
    ui.plainClockShadowColor->setColor(m_plainClockShadowColor);
    ui.configureDateFormats->setIcon( KIcon( "configure" ) );

    QStringList dateStyles;
    dateStyles << i18nc("A kind of date representation", "No date")
               << i18nc("A kind of date representation", "Compact date")
               << i18nc("A kind of date representation", "Short date")
               << i18nc("A kind of date representation", "Long date")
               << i18nc("A kind of date representation", "ISO date");

    ui.dateStyle->addItems(dateStyles);
    ui.dateStyle->setCurrentIndex(m_dateStyle);

    connect(ui.drawShadow, SIGNAL(toggled(bool)),
            this, SLOT(configDrawShadowToggled(bool)));
    connect(ui.configureDateFormats, SIGNAL(clicked()),
            this, SLOT(launchDateKcm()));
    configDrawShadowToggled(m_drawShadow);

    connect(ui.plainClockFont, SIGNAL(currentFontChanged(QFont)),
            parent, SLOT(settingsModified()));
    connect(ui.plainClockFontBold, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.plainClockFontItalic, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.useCustomColor, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.plainClockColor, SIGNAL(changed(QColor)),
            parent, SLOT(settingsModified()));
    connect(ui.drawShadow, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.useCustomShadowColor, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.plainClockShadowColor, SIGNAL(changed(QColor)), 
            parent, SLOT(settingsModified()));
    connect(ui.showTimeZone, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.secondsCheckbox, SIGNAL(stateChanged(int)),
            parent, SLOT(settingsModified()));
    connect(ui.dateStyle, SIGNAL(currentIndexChanged(int)),
            parent, SLOT(settingsModified()));
}

void Clock::configDrawShadowToggled(bool value)
{
    ui.useCustomShadowColor->setEnabled(value);
    ui.customShadowColorLabel->setEnabled(value);
    ui.plainClockShadowColor->setEnabled(value && ui.useCustomShadowColor->isChecked());
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();

    m_showTimezone = ui.showTimeZone->isChecked();
    cg.writeEntry("showTimezone", m_showTimezone);

    if (m_isDefaultFont && ui.plainClockFont->currentFont() != m_plainClockFont) {
        m_isDefaultFont = false;
    }
    m_plainClockFont = ui.plainClockFont->currentFont();

    //We need this to happen before we disconnect/reconnect sources to ensure
    //that the update interval is set properly.
    if (m_showSeconds != ui.secondsCheckbox->isChecked()) {
        m_showSeconds = !m_showSeconds;
        cg.writeEntry("showSeconds", m_showSeconds);

        if (m_showSeconds) {
            //We don't need to cache the applet if it update every second
            setCacheMode(QGraphicsItem::NoCache);
        } else {
            setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }

        changeEngineTimezone(currentTimezone(), currentTimezone());
    }

    m_dateStyle = ui.dateStyle->currentIndex();
    cg.writeEntry("dateStyle", m_dateStyle);

    m_showSeconds = ui.secondsCheckbox->checkState() == Qt::Checked;
    cg.writeEntry("showSeconds", m_showSeconds);

    m_useCustomColor = ui.useCustomColor->isChecked();
    cg.writeEntry("useCustomColor", m_useCustomColor);
    if (m_useCustomColor) {
        m_plainClockColor = ui.plainClockColor->color();
        cg.writeEntry("plainClockColor", m_plainClockColor);
        m_pixmap = QPixmap();
        delete m_svg;
        m_svg = 0;
    } else {
        m_plainClockColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    }

    m_useCustomShadowColor = ui.useCustomShadowColor->isChecked();
    cg.writeEntry("useCustomShadowColor", m_useCustomShadowColor);
    if (m_useCustomShadowColor) {
        m_plainClockShadowColor = ui.plainClockShadowColor->color();
        cg.writeEntry("plainClockShadowColor", m_plainClockShadowColor);
    } else {
        m_plainClockShadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    }
    m_drawShadow = ui.drawShadow->isChecked();
    cg.writeEntry("plainClockDrawShadow", m_drawShadow);

    m_plainClockFont.setBold(ui.plainClockFontBold->checkState() == Qt::Checked);
    m_plainClockFont.setItalic(ui.plainClockFontItalic->checkState() == Qt::Checked);
    cg.writeEntry("plainClockFont", m_plainClockFont);

    constraintsEvent(Plasma::SizeConstraint);
    generatePixmap();
    update();
    emit sizeHintChanged(Qt::PreferredSize);
    emit configNeedsSaving();
}

void Clock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    resetLastTimeSeen();
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
    if (m_dateStyle || showTimezone()) {
        QString dateString;

        //Create the localized date string if needed
        if (m_dateStyle) {
            // JPL This needs a complete rewrite for l10n issues
            QString day = KGlobal::locale()->calendar()->formatDate(m_date, KLocale::Day, KLocale::ShortNumber);
            QString month = KGlobal::locale()->calendar()->formatDate(m_date, KLocale::Month, KLocale::LongNumber);

            if (m_dateStyle == 1) {         //compact date
                dateString = i18nc("@label Compact date: "
                        "%1 day in the month, %2 month number",
                        "%1/%2", day, month);
            } else if (m_dateStyle == 2) {    //short date
                dateString = KGlobal::locale()->formatDate(m_date, KLocale::ShortDate);
            } else if (m_dateStyle == 3) {    //long date
                dateString = KGlobal::locale()->formatDate(m_date, KLocale::LongDate);
            } else if (m_dateStyle == 4) {    //ISO date
                dateString = KGlobal::locale()->formatDate(m_date, KLocale::IsoDate);
            } else {                          //shouldn't happen
                dateString = KGlobal::locale()->formatDate(m_date, KLocale::ShortDate);
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

        QPen datePen = p->pen();
        QColor dateColor = m_plainClockColor;
        dateColor.setAlphaF(0.7);
        datePen.setColor(dateColor);
        p->setPen(datePen);

        if (formFactor() == Plasma::Horizontal && (contentsRect.height() < smallFont.pointSize()*6)) {
            p->drawText(dateRect, Qt::TextSingleLine | Qt::AlignHCenter, m_dateString);
        } else {
            p->drawText(dateRect, Qt::TextWordWrap | Qt::AlignHCenter, m_dateString);
        }

        p->setFont(f);
    }

    if (m_useCustomColor || !m_svgExistsInTheme) {
        QFontMetrics fm(p->font());

        QPointF timeTextOrigin(QPointF(qMax(0, (m_timeRect.center().x() - fm.width(fakeTimeString) / 2)),
                            (m_timeRect.center().y() + fm.height() / 3)));
        p->translate(-0.5, -0.5);

        if (m_drawShadow) {
            QPen tmpPen = p->pen();

            // Paint a backdrop behind the time's text
            qreal shadowOffset = 1.0;
            QPen shadowPen;
            QColor shadowColor = m_plainClockShadowColor;
            shadowColor.setAlphaF(.4);
            shadowPen.setColor(shadowColor);
            p->setPen(shadowPen);
            QPointF shadowTimeTextOrigin = QPointF(timeTextOrigin.x() + shadowOffset,
                                                timeTextOrigin.y() + shadowOffset);
            p->drawText(shadowTimeTextOrigin, timeString);

            p->setPen(tmpPen);

            // Paint the time itself with a linear translucency gradient
            QLinearGradient gradient = QLinearGradient(QPointF(0, 0), QPointF(0, fm.height()));

            QColor startColor = m_plainClockColor;
            startColor.setAlphaF(.95);
            QColor stopColor = m_plainClockColor;
            stopColor.setAlphaF(.7);

            gradient.setColorAt(0.0, startColor);
            gradient.setColorAt(0.5, stopColor);
            gradient.setColorAt(1.0, startColor);
            QBrush gradientBrush(gradient);

            QPen gradientPen(gradientBrush, tmpPen.width());
            p->setPen(gradientPen);
        }
        p->drawText(timeTextOrigin, timeString);
    //when use the custom theme colors, draw the time textured
    } else {
        QRect adjustedTimeRect = m_pixmap.rect();
        adjustedTimeRect.moveCenter(m_timeRect.center());
        p->drawPixmap(adjustedTimeRect, m_pixmap);
    }
}

void Clock::generatePixmap()
{
    if (m_useCustomColor || !m_svgExistsInTheme) {
        return;
    }

    if (!m_svg) {
        m_svg = new Plasma::Svg(this);
        m_svg->setImagePath("widgets/labeltexture");
        m_svg->setContainsMultipleImages(true);
    }

    const QString fakeTimeString = KGlobal::locale()->formatTime(QTime(23,59,59), m_showSeconds);
    const QString timeString = KGlobal::locale()->formatTime(m_time, m_showSeconds);

    QRect rect(contentsRect().toRect());
    QFont font(m_plainClockFont);
    prepareFont(font, rect, fakeTimeString, true);
    m_pixmap = Plasma::PaintUtils::texturedText(timeString, font, m_svg);
}

void Clock::expandFontToMax(QFont &font, const QString &text)
{
    bool first = true;
    const QRect rect = contentsRect().toRect();

    // Starting with the given font, increase its size until it'll fill the rect
    do {
        if (first) {
            first = false;
        } else  {
            font.setPointSize(font.pointSize() + 1);
        }

        const QFontMetrics fm(font);
        QRect fr = fm.boundingRect(rect, Qt::TextSingleLine, text);
        if (fr.width() >= rect.width() || fr.height() >= rect.height()) {
            break;
        }
    } while (true);
}

void Clock::prepareFont(QFont &font, QRect &rect, const QString &text, bool singleline)
{
    QRect tmpRect;
    bool first = true;
    const int smallest = KGlobalSettings::smallestReadableFont().pointSize();

    // Starting with the given font, decrease its size until it'll fit in the
    // given rect allowing wrapping where possible
    do {
        if (first) {
            first = false;
        } else  {
            font.setPointSize(qMax(smallest, font.pointSize() - 1));
        }

        const QFontMetrics fm(font);
        int flags = (singleline || ((formFactor() == Plasma::Horizontal) && (contentsRect().height() < font.pointSize()*6))) ?
                    Qt::TextSingleLine : Qt::TextWordWrap;

        tmpRect = fm.boundingRect(rect, flags, text);
    } while (font.pointSize() > smallest &&
             (tmpRect.width() > rect.width() || tmpRect.height() > rect.height()));

    rect = tmpRect;
}

QRect Clock::preparePainter(QPainter *p, const QRect &rect, const QFont &font, const QString &text, bool singleline)
{
    QRect tmpRect = rect;
    QFont tmpFont = font;

    prepareFont(tmpFont, tmpRect, text, singleline);

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
    m_svgExistsInTheme = Plasma::Theme::defaultTheme()->currentThemeHasImage("widgets/labeltexture");

    if (!m_useCustomColor) {
        m_plainClockColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    }

    if (!m_useCustomShadowColor) {
        m_plainClockShadowColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    }

    if (!m_useCustomColor || !m_useCustomShadowColor) {
        update();
    }
}

void Clock::launchDateKcm() //SLOT
{
    KService::List offers = KServiceTypeTrader::self()->query("KCModule", "Library == 'kcm_locale'");
    if (!offers.isEmpty()) {
        KService::Ptr service = offers.first();
        KRun::run(*service, KUrl::List(), 0);
    }
    update();
}

#include "clock.moc"
