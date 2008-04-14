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

#include <plasma/svg.h>

Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Containment(parent, args),
      m_showTimeString(false),
      m_showSecondHand(false),
      m_secondHandUpdateTimer(0)
{
    setHasConfigurationInterface(true);
    resize(125, 125);
    setRemainSquare(true);

    m_theme = new Plasma::Svg("widgets/clock", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
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
    m_timezone = cg.readEntry("timezone", "Local");

    connectToEngine();
}

void Clock::connectToEngine()
{
    Plasma::DataEngine* timeEngine = dataEngine("time");
    if (m_showSecondHand) {
        timeEngine->connectSource(m_timezone, this, 500);
    } else {
        timeEngine->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
    }
}

void Clock::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        setDrawStandardBackground(false);
    }
}

QPainterPath Clock::shape() const
{
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

void Clock::createConfigurationInterface(KConfigDialog *parent)
{
    //TODO: Make the size settable
    QWidget *widget = new QWidget();
    ui.setupUi(widget);
    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));
    parent->addPage(widget, parent->windowTitle(), "chronometer");

    ui.timeZones->setSelected(m_timezone, true);
    ui.timeZones->setEnabled(m_timezone != "Local");
    ui.localTimeZone->setChecked(m_timezone == "Local");
    ui.showTimeStringCheckBox->setChecked(m_showTimeString);
    ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();
    m_showTimeString = ui.showTimeStringCheckBox->isChecked();
    m_showSecondHand = ui.showSecondHandCheckBox->isChecked();

    cg.writeEntry("showTimeString", m_showTimeString);
    cg.writeEntry("showSecondHand", m_showSecondHand);
    update();
    QStringList tzs = ui.timeZones->selection();

    if (ui.localTimeZone->checkState() == Qt::Checked) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        cg.writeEntry("timezone", m_timezone);
    } else if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            m_timezone = tz;
            cg.writeEntry("timezone", m_timezone);
        }
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
        cg.writeEntry("timezone", m_timezone);
    }

    connectToEngine();
    constraintsUpdated(Plasma::AllConstraints);
    emit configNeedsSaving();
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
    const QSize elementSize = m_theme->elementSize(handName);

    p->translate(boundSize.width() / 2, boundSize.height() / 2);
    p->rotate(rotation);
    p->translate(-elementSize.width() / 2, -elementSize.width());
    m_theme->paint(p, QRect(QPoint(0, 0), elementSize), handName);
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

    drawHand(p, hours, "HourHand");
    drawHand(p, minutes, "MinuteHand");

    //Make sure we paint the second hand on top of the others
    if (m_showSecondHand) {
        static const double anglePerSec = 6;
        qreal seconds = anglePerSec * m_time.second() - 180;

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

        drawHand(p, seconds, "SecondHand");
    }

    p->save();
    m_theme->resize(boundSize);
    elementSize = m_theme->elementSize("HandCenterScrew");
    tempRect.setSize(elementSize);
    p->translate(boundSize.width() / 2 - elementSize.width() / 2, boundSize.height() / 2 - elementSize.height() / 2);
    m_theme->paint(p, tempRect, "HandCenterScrew");
    p->restore();

    if (m_showTimeString) {
        if (m_showSecondHand) {
            //FIXME: temporary time output
            QString time = m_time.toString();
            QFontMetrics fm(QApplication::font());
            p->drawText((rect.width()/2 - fm.width(time) / 2),
                        ((rect.height()/2) - fm.xHeight()*3), m_time.toString());
        } else {
            QString time = m_time.toString("hh:mm");
            QFontMetrics fm(QApplication::font());
            p->drawText((rect.width()/2 - fm.width(time) / 2),
                        ((rect.height()/2) - fm.xHeight()*3), m_time.toString("hh:mm"));
        }
    }

    m_theme->paint(p, rect, "Glass");
}

#include "clock.moc"
