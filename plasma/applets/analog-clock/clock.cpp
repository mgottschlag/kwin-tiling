/***************************************************************************
 *   Copyright 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>          *
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

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>

#include <plasma/svg.h>

Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_showTimeString(false),
      m_showSecondHand(false),
      m_timezone("Local"),
      m_dialog(0)
{
    setHasConfigurationInterface(true);
    setContentSize(125, 125);
    setMinimumContentSize(50, 50);

    m_theme = new Plasma::Svg("widgets/clock", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(size());
}

void Clock::init()
{
    KConfigGroup cg = config();
    m_showTimeString = cg.readEntry("showTimeString", false);
    m_showSecondHand = cg.readEntry("showSecondHand", false);
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

Qt::Orientations Clock::expandingDirections() const
{
    return 0;
}

QSizeF Clock::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    switch (formFactor()) {
        case Plasma::Vertical:
            sizeHint.setHeight(sizeHint.width());
            break;
        case Plasma::Horizontal:
            sizeHint.setWidth(sizeHint.height());
            break;
        default:
            break;
    }

    return sizeHint;
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
    m_lastTimeSeen = m_time;
    update();
}

void Clock::showConfigurationInterface() //TODO: Make the size settable
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18nc("@title:window","Configure Clock") );

        QWidget *widget = new QWidget;
        ui.setupUi(widget);
        m_dialog->setMainWidget(widget);
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

    }

    ui.timeZones->setSelected(m_timezone, true);
    ui.showTimeStringCheckBox->setChecked(m_showTimeString);
    ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
    m_dialog->show();
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

    if (tzs.count() > 0) {
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
    }

    connectToEngine();
    constraintsUpdated(Plasma::AllConstraints);
    cg.config()->sync();
}

Clock::~Clock()
{
}

void Clock::drawHand(QPainter *p, int rotation, const QString &handName)
{
    Q_UNUSED(p);
    Q_UNUSED(rotation);
    Q_UNUSED(handName);
// TODO: IMPLEMENT ME!
//     p->save();
//     QRectF tempRect(0, 0, 0, 0);
//     QSizeF boundSize = boundingRect().size();
//     QSize elementSize;
// 
//     p->translate(boundSize.width()/2, boundSize.height()/2);
//     p->rotate(rotation);
//     elementSize = m_theme->elementSize(handName);
//     if (scaleFactor != 1) {
//         elementSize = QSize(elementSize.width()*scaleFactor, elementSize.height()*scaleFactor);
//     }
//     p->translate(-elementSize.width()/2, -elementSize.width());
//     m_theme->resize(elementSize);
//     tempRect.setSize(elementSize);
//     m_theme->paint(p, tempRect, handName);
//     p->restore();
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Q_UNUSED(option)
    Q_UNUSED(rect)

    QRectF tempRect(0, 0, 0, 0);

    QSizeF boundSize = contentSize();
    QSize elementSize;

    p->setRenderHint(QPainter::SmoothPixmapTransform);

    qreal seconds = 6.0 * m_time.second() - 180;
    qreal minutes = 6.0 * m_time.minute() - 180;
    qreal hours = 30.0 * m_time.hour() - 180 + ((m_time.minute() / 59.0) * 30.0);

    m_theme->paint(p, rect, "ClockFace");

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(hours);
    elementSize = m_theme->elementSize("HourHand");

    p->translate(-elementSize.width()/2, -elementSize.width());
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "HourHand");
    p->restore();

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(minutes);
    elementSize = m_theme->elementSize("MinuteHand");
    elementSize = QSize(elementSize.width(), elementSize.height());
    p->translate(-elementSize.width()/2, -elementSize.width());
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "MinuteHand");
    p->restore();

    //Make sure we paint the second hand on top of the others
    if (m_showSecondHand) {
        p->save();
        p->translate(boundSize.width()/2, boundSize.height()/2);
        p->rotate(seconds);
        elementSize = m_theme->elementSize("SecondHand");
        elementSize = QSize(elementSize.width(), elementSize.height());
        p->translate(-elementSize.width()/2, -elementSize.width());
        tempRect.setSize(elementSize);
        m_theme->paint(p, tempRect, "SecondHand");
        p->restore();
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
            p->drawText((int)(rect.width()/2 - fm.width(time) / 2),
                        (int)((rect.height()/2) - fm.xHeight()*3), m_time.toString());
        } else {
            QString time = m_time.toString("hh:mm");
            QFontMetrics fm(QApplication::font());
            p->drawText((int)(rect.width()/2 - fm.width(time) / 2),
                        (int)((rect.height()/2) - fm.xHeight()*3), m_time.toString("hh:mm"));
        }
    }

    m_theme->paint(p, rect, "Glass");
}

#include "clock.moc"
