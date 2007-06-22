/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <ruphy@fsfe.org>             *
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

Clock::Clock(QObject *parent, const QStringList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setFlags(QGraphicsItem::ItemIsMovable);

    KConfigGroup cg = appletConfig();
    m_showTimeString = cg.readEntry("showTimeString", false);
    m_showSecondHand = cg.readEntry("showSecondHand", false);
    m_pixelSize = cg.readEntry("size", 250);
    m_timezone = cg.readEntry("timezone", i18n("Local"));
    m_theme = new Plasma::Svg("widgets/clock", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(m_pixelSize, m_pixelSize);

    Plasma::DataEngine* timeEngine = dataEngine("time");
    timeEngine->connectSource(m_timezone, this);
    timeEngine->setProperty("reportSeconds", m_showSecondHand);
    constraintsUpdated();
}

QRectF Clock::boundingRect() const
{
    return m_bounds;
}

void Clock::constraintsUpdated()
{
    prepareGeometryChange();
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        QSize s = m_theme->size();
        m_bounds = QRect(0, 0, s.width(), s.height());
    } else {
        QFontMetrics fm(QApplication::font());
        m_bounds = QRectF(0, 0, fm.width("00:00:00") * 1.2, fm.height() * 1.5);
    }
}

void Clock::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data[i18n("Time")].toTime();

    if (m_time.minute() == m_lastTimeSeen.minute() &&
        m_time.second() == m_lastTimeSeen.second()) {
        // avoid unnecessary repaints
        //kDebug() << "avoided unecessary update!" << endl;
        return;
    }

    m_lastTimeSeen = m_time;
    update();
}

void Clock::configureDialog() //TODO: Make the size settable
{
     if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( "Configure Clock" );

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

        ui.timeZones->setSelected(m_timezone, true);
        ui.spinSize->setValue((int)m_bounds.width());
        ui.showTimeStringCheckBox->setChecked(m_showTimeString);
        ui.showSecondHandCheckBox->setChecked(m_showSecondHand);
    }
    m_dialog->show();
}

void Clock::configAccepted()
{
    KConfigGroup cg = appletConfig();
    m_showTimeString = ui.showTimeStringCheckBox->checkState() == Qt::Checked;
    m_showSecondHand = ui.showSecondHandCheckBox->checkState() == Qt::Checked;
    cg.writeEntry("showTimeString", m_showTimeString);
    cg.writeEntry("showSecondHand", m_showSecondHand);
    dataEngine("time")->setProperty("reportSeconds", m_showSecondHand);
    QGraphicsItem::update();
    cg.writeEntry("size", ui.spinSize->value());
    m_theme->resize(ui.spinSize->value(), ui.spinSize->value());
    QStringList tzs = ui.timeZones->selection();
    /*
    if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            m_timezone = tz;
            dataEngine("time")->connectSource(m_timezone, this);
        }
    } else if (m_timezone != i18n("Local")) {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = i18n("Local");
        dataEngine("time")->connectSource(m_timezone, this);
    }
    */

    dataEngine("time")->connectSource(m_timezone, this);
    constraintsUpdated();
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

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF tempRect(0, 0, 0, 0);
    QRectF boundRect = boundingRect();

    QSizeF boundSize = boundRect.size();
    QSize elementSize;

    p->setRenderHint(QPainter::SmoothPixmapTransform);

    qreal seconds = 6.0 * m_time.second() - 180;
    qreal minutes = 6.0 * m_time.minute() - 180;
    qreal hours = 30.0 * m_time.hour() - 180 + ((m_time.minute() / 59.0) * 30.0);

    if (formFactor() == Plasma::Horizontal ||
        formFactor() == Plasma::Vertical) {
        QString time = m_time.toString();
        QFontMetrics fm(QApplication::font());
        p->drawText((int)(boundRect.width() * 0.1), (int)(boundRect.height() * 0.25), m_time.toString());
        return;
    }
    m_theme->paint(p, boundRect, "ClockFace");

    p->save();
    p->translate(boundSize.width()/2, boundSize.height()/2);
    p->rotate(hours);
    elementSize = m_theme->elementSize("HourHand");

    p->translate(-elementSize.width()/2, -elementSize.width());
    tempRect.setSize(elementSize);
    m_theme->paint(p, tempRect, "HourHand");
    p->restore();

//     drawHand(p, hours, "SecondHand", 1);
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
        //FIXME: temporary time output
        QString time = m_time.toString();
        QFontMetrics fm(QApplication::font());
        p->drawText((int)(boundRect.width()/2 - fm.width(time) / 2),
                    (int)((boundRect.height()/2) - fm.xHeight()*3), m_time.toString());
    }

    m_theme->paint(p, boundRect, "Glass");
}

#include "clock.moc"
