/***************************************************************************
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>               *
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

#include <QGraphicsScene>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QCheckBox>
#include <QPushButton>

#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KIconLoader>
#include <KSharedConfig>

#include <Plasma/Dialog>
#include <Plasma/FrameSvg>
#include <Plasma/PaintUtils>
#include <Plasma/Svg>
#include <Plasma/Theme>

Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_showSecondHand(false),
      m_showTimezoneString(false),
      m_showingTimezone(false),
      m_tzFrame(0),
      m_repaintCache(RepaintAll),
      m_faceCache(QPixmap()),
      m_handsCache(QPixmap()),
      m_glassCache(QPixmap()),
      m_secondHandUpdateTimer(0),
      m_animateSeconds(false)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");
    // this catalog is only used once on the first start of the clock to translate the timezone in the configuration file
    KGlobal::locale()->insertCatalog("timezones4");
    setHasConfigurationInterface(true);
    resize(256, 256);
    setAspectRatioMode(Plasma::Square);
    setBackgroundHints(NoBackground);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/clock");
    m_theme->setContainsMultipleImages(true);
    m_theme->resize(size());

    connect(m_theme, SIGNAL(repaintNeeded()), this, SLOT(repaintNeeded()));
}

Clock::~Clock()
{
}

void Clock::init()
{
    ClockApplet::init();
    m_oldTimezone = currentTimezone();
    configChanged();
}

void Clock::connectToEngine()
{
    resetLastTimeSeen();

    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->disconnectSource(m_oldTimezone, this);
    m_oldTimezone = currentTimezone();

    if (m_showSecondHand) {
        timeEngine->connectSource(currentTimezone(), this, 500);
    } else {
        timeEngine->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);
    }
}

void Clock::clockConfigChanged()
{
    KConfigGroup cg = config();
    m_showSecondHand = cg.readEntry("showSecondHand", false);
    m_showTimezoneString = cg.readEntry("showTimezoneString", false);
    m_showingTimezone = m_showTimezoneString;
    m_fancyHands = cg.readEntry("fancyHands", false);
    setCurrentTimezone(cg.readEntry("timezone", localTimezone()));

    if (m_showSecondHand) {
        //We don't need to cache the applet if it update every seconds
        setCacheMode(QGraphicsItem::NoCache);
    } else {
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    connectToEngine();
    invalidateCache();
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    ClockApplet::constraintsEvent(constraints);

    if (constraints & Plasma::SizeConstraint) {
        invalidateCache();
    }

    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar || formFactor() == Plasma::MediaCenter) {
            setPreferredSize(256, 256);
        } else {
            setPreferredSize(-1, -1);
        }
    }
}

QPainterPath Clock::shape() const
{
    if (m_theme->hasElement("hint-square-clock")) {
        return Applet::shape();
    }

    QPainterPath path;
    // we adjust by 2px all around to allow for smoothing the jaggies
    // if the ellipse is too small, we'll get a nastily jagged edge around the clock
    path.addEllipse(boundingRect().adjusted(-2, -2, 2, 2));
    return path;
}

void Clock::dataUpdated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();

    if (m_time.minute() == lastTimeSeen().minute() &&
        (!m_showSecondHand || m_time.second() == lastTimeSeen().second())) {
        // avoid unnecessary repaints
        return;
    }

    if (m_time.minute() != lastTimeSeen().minute() && m_repaintCache == RepaintNone) {
        m_repaintCache = RepaintHands;
    }

    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        updateTipContent();
    }

    if (m_secondHandUpdateTimer) {
        m_secondHandUpdateTimer->stop();
    }

    m_animateSeconds = true;

    updateClockApplet(data);
    update();
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("Appearance"), "view-media-visualization");

    ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
    ui.showTimezoneStringCheckBox->setChecked(m_showTimezoneString);

    connect(ui.showSecondHandCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
    connect(ui.showTimezoneStringCheckBox, SIGNAL(stateChanged(int)), parent, SLOT(settingsModified()));
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    m_showTimezoneString = ui.showTimezoneStringCheckBox->isChecked();
    m_showingTimezone = m_showTimezoneString || shouldDisplayTimezone();
    m_showSecondHand = ui.showSecondHandCheckBox->isChecked();

    if (m_showSecondHand) {
        //We don't need to cache the applet if it update every seconds
        setCacheMode(QGraphicsItem::NoCache);
    } else {
        setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }

    cg.writeEntry("showSecondHand", m_showSecondHand);
    cg.writeEntry("showTimezoneString", m_showTimezoneString);
    update();

    dataEngine("time")->disconnectSource(currentTimezone(), this);
    connectToEngine();

    constraintsEvent(Plasma::AllConstraints);
    emit configNeedsSaving();
}

void Clock::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    dataEngine("time")->disconnectSource(oldTimezone, this);
    Plasma::DataEngine* timeEngine = dataEngine("time");

    if (m_showSecondHand) {
        timeEngine->connectSource(newTimezone, this, 500);
    } else {
        timeEngine->connectSource(newTimezone, this, 6000, Plasma::AlignToMinute);
    }

    if (m_showingTimezone != (m_showTimezoneString || shouldDisplayTimezone())) {
        m_showingTimezone = !m_showingTimezone;
        constraintsEvent(Plasma::SizeConstraint);
    }
    m_repaintCache = RepaintAll;
}

void Clock::repaintNeeded()
{
    m_repaintCache = RepaintAll;
    update();
}

void Clock::moveSecondHand()
{
    //kDebug() << "moving second hand";
    update();
}

void Clock::drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName)
{
    // this code assumes the following conventions in the svg file:
    // - the _vertical_ position of the hands should be set with respect to the center of the face
    // - the _horizontal_ position of the hands does not matter
    // - the _shadow_ elements should have the same vertical position as their _hand_ element counterpart

    QRectF elementRect;
    QString name = handName + "HandShadow";
    if (m_theme->hasElement(name)) {
        p->save();

        elementRect = m_theme->elementRect(name);
        if( rect.height() < KIconLoader::SizeEnormous )
            elementRect.setWidth( elementRect.width() * 2.5 );
        static const QPoint offset = QPoint(2, 3);

        p->translate(rect.x() + (rect.width() / 2) + offset.x(), rect.y() + (rect.height() / 2) + offset.y());
        p->rotate(rotation);
        p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
        m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

        p->restore();
    }

    p->save();

    name = handName + "Hand";
    elementRect = m_theme->elementRect(name);
    if (rect.height() < KIconLoader::SizeEnormous) {
        elementRect.setWidth(elementRect.width() * 2.5);
    }

    p->translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
    p->rotate(rotation);
    p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

    p->restore();
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)

    // compute hand angles
    const qreal minutes = 6.0 * m_time.minute() - 180;
    const qreal hours = 30.0 * m_time.hour() - 180 +
                        ((m_time.minute() / 59.0) * 30.0);
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * m_time.second() - 180;

        if (m_fancyHands) {
            if (!m_secondHandUpdateTimer) {
                m_secondHandUpdateTimer = new QTimer(this);
                connect(m_secondHandUpdateTimer, SIGNAL(timeout()), this, SLOT(moveSecondHand()));
            }

            if (m_animateSeconds && !m_secondHandUpdateTimer->isActive()) {
                //kDebug() << "starting second hand movement";
                m_secondHandUpdateTimer->start(50);
                m_animationStart = QTime::currentTime().msec();
            } else {
                static const int runTime = 500;
                static const double m = 1; // Mass
                static const double b = 1; // Drag coefficient
                static const double k = 1.5; // Spring constant
                static const double PI = 3.141592653589793; // the universe is irrational
                static const double gamma = b / (2 * m); // Dampening constant
                static const double omega0 = sqrt(k / m);
                static const double omega1 = sqrt(omega0 * omega0 - gamma * gamma);
                const double elapsed = QTime::currentTime().msec() - m_animationStart;
                const double t = (4 * PI) * (elapsed / runTime);
                const double val = 1 + exp(-gamma * t) * -cos(omega1 * t);

                if (elapsed > runTime) {
                    m_secondHandUpdateTimer->stop();
                    m_animateSeconds = false;
                } else {
                    seconds += -anglePerSec + (anglePerSec * val);
                }
            }
        } else {
            if (!m_secondHandUpdateTimer) {
                m_secondHandUpdateTimer = new QTimer(this);
                connect(m_secondHandUpdateTimer, SIGNAL(timeout()), this, SLOT(moveSecondHand()));
            }

            if (m_animationStart != seconds && !m_secondHandUpdateTimer->isActive()) {
                m_secondHandUpdateTimer->start(50);
                m_animationStart = seconds; //we don't want to have a second animation if there is a external update (wallpaper etc).
                seconds += 1;
            } else {
                m_secondHandUpdateTimer->stop();
            }
        }
    }

    if (contentsRect().size().toSize() != m_theme->size()) {
        invalidateCache();
    }

    // paint face and glass cache
    QRect faceRect = m_faceCache.rect();
    if (m_repaintCache == RepaintAll) {
        m_faceCache.fill(Qt::transparent);
        m_glassCache.fill(Qt::transparent);

        QPainter facePainter(&m_faceCache);
        QPainter glassPainter(&m_glassCache);
        facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
        glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        m_theme->paint(&facePainter, faceRect, "ClockFace");

        glassPainter.save();
        QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize("HandCenterScrew"));
        glassPainter.translate(faceRect.width() / 2 - elementRect.width() / 2, faceRect.height() / 2 - elementRect.height() / 2);
        m_theme->paint(&glassPainter, elementRect, "HandCenterScrew");
        glassPainter.restore();

        m_theme->paint(&glassPainter, faceRect, "Glass");

        // get vertical translation, see drawHand() for more details
        m_verticalTranslation = m_theme->elementRect("ClockFace").center().y();
    }

    // paint hour and minute hands cache
    if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
        m_handsCache.fill(Qt::transparent);

        QPainter handsPainter(&m_handsCache);
        handsPainter.drawPixmap(faceRect, m_faceCache, faceRect);
        handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        drawHand(&handsPainter, faceRect, m_verticalTranslation, hours, "Hour");
        drawHand(&handsPainter, faceRect, m_verticalTranslation, minutes, "Minute");
    }

    // reset repaint cache flag
    m_repaintCache = RepaintNone;

    // paint caches and second hand
    QRect targetRect = faceRect;
    if (targetRect.width() < rect.width()) {
        targetRect.moveLeft((rect.width() - targetRect.width()) / 2);
    }
    targetRect.translate(rect.topLeft());

    p->drawPixmap(targetRect, m_handsCache, faceRect);
    if (m_showSecondHand) {
        p->setRenderHint(QPainter::SmoothPixmapTransform);
        drawHand(p, targetRect, m_verticalTranslation, seconds, "Second");
    }
    p->drawPixmap(targetRect, m_glassCache, faceRect);

    // optionally paint the time string
    if (m_showingTimezone) {
        QString time = prettyTimezone();

        if (!time.isEmpty()) {
            QRect textRect = tzRect(time);
            tzFrame()->paintFrame(p, textRect, QRect(QPoint(0, 0), textRect.size()));

            qreal left, top, right, bottom;
            tzFrame()->getMargins(left, top, right, bottom);

            p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
            p->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
            p->drawText(textRect.adjusted(left, 0, -right, 0), Qt::AlignCenter, time);
        }
    }
}

QRect Clock::tzRect(const QString &text)
{
    QRect rect = contentsRect().toRect();

    QFont font = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    QFontMetrics fontMetrics(font);

    qreal left, top, right, bottom;
    tzFrame()->getMargins(left, top, right, bottom);

    int width = left + right + fontMetrics.width(text) + fontMetrics.averageCharWidth() * 2;
    int height = top + bottom + fontMetrics.height();
    width = qMin(width, rect.width());
    height = qMin(height, rect.height());

    return QRect((rect.width() - width) / 2, rect.bottom() - height, width, height);
}

Plasma::FrameSvg *Clock::tzFrame()
{
    if (!m_tzFrame) {
        m_tzFrame = new Plasma::FrameSvg(this);
        m_tzFrame->setImagePath("widgets/background");
    }

    return m_tzFrame;
}

void Clock::invalidateCache()
{
    m_repaintCache = RepaintAll;

    QSize pixmapSize = contentsRect().size().toSize();
    const int squareEdge = qMin(pixmapSize.width(), pixmapSize.height());

    if (m_showingTimezone) {
        QRect tzArea = tzRect(prettyTimezone());
        pixmapSize.setHeight(qMax(10, squareEdge - tzArea.height()));
        tzFrame()->resizeFrame(tzArea.size());
    }

    pixmapSize.setWidth(pixmapSize.height());
    m_faceCache = QPixmap(pixmapSize);
    m_handsCache = QPixmap(pixmapSize);
    m_glassCache = QPixmap(pixmapSize);

    m_faceCache.fill(Qt::transparent);
    m_glassCache.fill(Qt::transparent);
    m_handsCache.fill(Qt::transparent);

    m_theme->resize(pixmapSize);
}

#include "clock.moc"
