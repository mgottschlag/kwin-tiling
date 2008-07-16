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

#include <QApplication>
#include <QBitmap>
#include <QGraphicsScene>
#include <QMatrix>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QStyleOptionGraphicsItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>

#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>

#include <plasma/dialog.h>
#include <plasma/paintutils.h>
#include <plasma/svg.h>
#include <plasma/theme.h>

Clock::Clock(QObject *parent, const QVariantList &args)
    : ClockApplet(parent, args),
      m_showTimeString(false),
      m_showSecondHand(false),
      m_secondHandUpdateTimer(0)
{
    KGlobal::locale()->insertCatalog("libplasmaclock");

    setHasConfigurationInterface(true);
    resize(125, 125);
    setAspectRatioMode(Plasma::Square);

    m_theme = new Plasma::Svg(this);
    m_theme->setImagePath("widgets/clock");
    m_theme->setContainsMultipleImages(false);
    m_theme->resize(size());
}

Clock::~Clock()
{
}

void Clock::init()
{
    KConfigGroup cg = config();
    m_showTimeString = cg.readEntry("showTimeString", false);
    m_showSecondHand = cg.readEntry("showSecondHand", false);
    m_fancyHands = cg.readEntry("fancyHands", false);
    setCurrentTimezone(cg.readEntry("timezone", localTimezone()));

    connectToEngine();
}

void Clock::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");
    if (m_showSecondHand) {
        timeEngine->connectSource(currentTimezone(), this, 500);
    } else {
        timeEngine->connectSource(currentTimezone(), this, 6000, Plasma::AlignToMinute);
    }
}

void Clock::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setBackgroundHints(NoBackground);
    }

    if (constraints & Plasma::SizeConstraint) {
        m_theme->resize(size());
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

    if (m_time.minute() == m_lastTimeSeen.minute() &&
        m_time.second() == m_lastTimeSeen.second()) {
        // avoid unnecessary repaints
        return;
    }

    if (m_secondHandUpdateTimer) {
        m_secondHandUpdateTimer->stop();
    }

    m_lastTimeSeen = m_time;
    update();
}

void Clock::createClockConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->addPage(widget, i18n("General"), icon());

    ui.showTimeStringCheckBox->setChecked(m_showTimeString);
    ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
}

void Clock::clockConfigAccepted()
{
    KConfigGroup cg = config();
    m_showTimeString = ui.showTimeStringCheckBox->isChecked();
    m_showSecondHand = ui.showSecondHandCheckBox->isChecked();

    cg.writeEntry("showTimeString", m_showTimeString);
    cg.writeEntry("showSecondHand", m_showSecondHand);
    update();

    dataEngine("time")->disconnectSource(currentTimezone(), this);
    connectToEngine();

    //TODO: why we don't call updateConstraints()?
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
}

void Clock::moveSecondHand()
{
    //kDebug() << "moving second hand";
    update();
}

void Clock::drawHand(QPainter *p, qreal rotation, const QString &handName)
{
    p->save();
    const QSizeF boundSize = boundingRect().size();
    const QRectF elementRect = m_theme->elementRect(handName);

    p->translate(boundSize.width() / 2, boundSize.height() / 2);
    p->rotate(rotation);
    p->translate(-elementRect.width() / 2, -(m_theme->elementRect("clockFace").center().y() - elementRect.top()) );
    m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), handName);
   
    p->restore();
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)
    Q_UNUSED(rect)

    QRectF tempRect(0, 0, 0, 0);

    QSizeF boundSize = geometry().size();
    QSize elementSize;

    p->setRenderHint(QPainter::SmoothPixmapTransform);

    const qreal minutes = 6.0 * m_time.minute() - 180;
    const qreal hours = 30.0 * m_time.hour() - 180 +
                        ((m_time.minute() / 59.0) * 30.0);

    m_theme->paint(p, rect, "ClockFace");

    
    //optionally paint the time string
    if (m_showTimeString) {
        QString time;
        QFontMetrics fm(QApplication::font());
        const int margin = 4;

        if (m_showSecondHand) {
            //FIXME: temporary time output
            time = m_time.toString();
        } else {
            time = m_time.toString("hh:mm");
        }

        QRect textRect((rect.width()/2 - fm.width(time) / 2),((rect.height()/2) - fm.xHeight()*4),
              fm.width(time), fm.xHeight());

        p->setPen(Qt::NoPen);
        QColor background = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
        background.setAlphaF(0.5);
        p->setBrush(background);

        p->setRenderHint(QPainter::Antialiasing, true);
        p->drawPath(Plasma::PaintUtils::roundedRectangle(textRect.adjusted(-margin, -margin, margin, margin), margin));
        p->setRenderHint(QPainter::Antialiasing, false);

        p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        
        p->drawText(textRect.bottomLeft(), time);
    }


    //Make sure we paint the second hand on top of the others
    qreal seconds = 0;
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        seconds = anglePerSec * m_time.second() - 180;

        if (m_fancyHands) {
            if (!m_secondHandUpdateTimer) {
                m_secondHandUpdateTimer = new QTimer(this);
                connect(m_secondHandUpdateTimer, SIGNAL(timeout()), this, SLOT(moveSecondHand()));
            }

            if (!m_secondHandUpdateTimer->isActive()) {
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
                } else {
                    seconds += -anglePerSec + (anglePerSec * val);
                }
            }
        }
    }

    if (m_theme->hasElement("HourHandShadow")) {
        p->translate(1,3);

        drawHand(p, hours, "HourHandShadow");
        drawHand(p, minutes, "MinuteHandShadow");

        if (m_showSecondHand) {
            drawHand(p, seconds, "SecondHandShadow");
        }

        p->translate(-1,-3);
    }
    
    drawHand(p, hours, "HourHand");
    drawHand(p, minutes, "MinuteHand");
    if (m_showSecondHand) {
        drawHand(p, seconds, "SecondHand");
    }

    p->save();
    elementSize = m_theme->elementSize("HandCenterScrew");
    tempRect.setSize(elementSize);
    p->translate(boundSize.width() / 2 - elementSize.width() / 2, boundSize.height() / 2 - elementSize.height() / 2);
    m_theme->paint(p, tempRect, "HandCenterScrew");
    p->restore();

    m_theme->paint(p, rect, "Glass");
}

#include "clock.moc"
