/***************************************************************************
 *   Copyright (C) 2005,2006,2007 by Siraj Razick <siraj@kdemail.net>      *
 *   Copyright (C) 2007 by Riccardo Iaconelli <riccardo@kde.org>           *
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
#include "clocknumber.h"

#include <math.h>

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QSpinBox>
#include <QTimeLine>
#include <KGlobalSettings>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KSharedConfig>
#include <KTimeZoneWidget>
#include <KDialog>

#include <plasma/svg.h>

#include <clocknumber.h>


Clock::Clock(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_dialog(0)
{
    setHasConfigurationInterface(true);
    setDrawStandardBackground(true);

    KConfigGroup cg = config();
    m_timezone = cg.readEntry("timezone", "Local");
    m_theme = new Plasma::Svg("widgets/digital-clock", this);
    m_theme->setContentType(Plasma::Svg::ImageSet);

    // take the size of the top half of the '0' number 
    // to calculate the aspect ratio when drawing
    m_defaultElementSize = m_theme->elementSize("e0-p1");

    Plasma::DataEngine* timeEngine = dataEngine("time");
//     timeEngine->connectSource(m_timezone, this, 6000, Plasma::AlignToMinute);
    timeEngine->connectSource(m_timezone, this, 100);
}

Qt::Orientations Clock::expandingDirections() const
{
    return Qt::Vertical;
}

QSizeF Clock::contentSizeHint() const
{
    return m_sizeHint; //FIXME: am I unimplemented?! :O
}

void Clock::constraintsUpdated(Plasma::Constraints)
{
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        m_sizeHint = QSize(150, 72);
    } else {
        kDebug() << "####################################### Small FormFactor";
        m_sizeHint = QSize(100, 48);
    }
    updateGeometry();
}

void Clock::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    m_time = data["Time"].toTime();
    m_date = data["Date"].toDate();

    if (m_time.minute() == m_lastTimeSeen.minute()) {
        // avoid unnecessary repaints
        // kDebug() << "avoided unecessary update!";
//         return; FIXME REENABLE ME
    }

    m_lastTimeSeen = m_time;

    animateUpdate();
}

void Clock::showConfigurationInterface()
{
    if (m_dialog == 0) {
        m_dialog = new KDialog;
        m_dialog->setCaption( i18n("Configure Clock") );

        ui.setupUi(m_dialog->mainWidget());
        m_dialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
        connect( m_dialog, SIGNAL(applyClicked()), this, SLOT(configAccepted()) );
        connect( m_dialog, SIGNAL(okClicked()), this, SLOT(configAccepted()) );

        ui.timeZones->setSelected(m_timezone, true);
    }

    m_dialog->show();
}

void Clock::configAccepted()
{
    KConfigGroup cg = config();
    QGraphicsItem::update();
    QStringList tzs = ui.timeZones->selection();

    if (tzs.count() > 0) {
        //TODO: support multiple timezones
        QString tz = tzs.at(0);
        if (tz != m_timezone) {
            dataEngine("time")->disconnectSource(m_timezone, this);
            m_timezone = tz;
            dataEngine("time")->connectSource(m_timezone, this);
        }
    } else if (m_timezone != "Local") {
        dataEngine("time")->disconnectSource(m_timezone, this);
        m_timezone = "Local";
    //m_theme->paint(p, QRectF(122, 40, 25, 40), "m"+month);
    //m_theme->paint(p, QRectF(152, 40, 25, 40), "d"+day[0]);
    //m_theme->paint(p, QRectF(152, 40, 25, 40), "d"+day[1]);
        dataEngine("time")->connectSource(m_timezone, this);
    }

    dataEngine("time")->connectSource(m_timezone, this);
    updateConstraints();
    cg.config()->sync();
}

Clock::~Clock()
{
}

void Clock::animateUpdate()
{
//     kDebug() << "animateUpdate() called";
    QTimeLine *tl = new QTimeLine(100, this);
    tl->setFrameRange(0, 4);
//     tl->setCurveShape(QTimeLine::EaseInCurve),
    connect(tl, SIGNAL(frameChanged(int)), this, SLOT(animationSlot(int)));
    tl->start();
}

void Clock::animationSlot(int step)
{
    if (step == 4) { // The animation is stopped
        m_animating = false;
        m_lastTimeSeen = m_time;
        update();
    } else {
        m_animating = true;
        m_animationStep = step;
        update();
    }
}

int Clock::calculateLeftOffset(int digitNumber)
{
    return 0;
}

void Clock::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED( option );

    if (!m_time.isValid() || !m_date.isValid()) {
        return;
    }

    p->setFont(KGlobalSettings::smallestReadableFont());

    const QString hours = m_time.toString("HH");
    const QString minutes = m_time.toString("ss");
    const QString day = m_date.toString("dd");
    const QString month = m_date.toString("MMM");
    const QString year = m_date.toString("yyyy");

    const qreal margin = 4;

    const int elHorizontalSpacing = 2;
    const int elVerticalSpacing = 2;
    int elWidth = qRound((contentsRect.width() - elHorizontalSpacing - margin*2) / 4.0);
    int elHeight = qRound((contentsRect.height() - elVerticalSpacing - margin*2) / 2.0);

    // enforce natural aspect ratio for elements
    QSize elSize = m_defaultElementSize;
    elSize.scale(elWidth, elHeight, Qt::KeepAspectRatio);
    elWidth = elSize.width();
    elHeight = elSize.height();

    // set left offset of clock elements so as to horizontally center the time display
    int leftOffset = (contentsRect.width() - (elWidth*4 + elHorizontalSpacing*4))/2.0;
    int upperElementTop = margin;
    int bottomElementTop = upperElementTop + elHeight + elVerticalSpacing;

    // update graphic sizes when the applet's size changes
    if ( contentsRect.size() != m_contentSize ) {
        m_theme->resize(elWidth,elHeight);
        m_contentSize = contentsRect.size();
    }

    // 10-hours-digit
    m_theme->paint(p, QRectF(leftOffset, upperElementTop, elWidth, elHeight), 'e'+hours[0]+"-p1");
    m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight), 'e'+hours[0]+"-p2");

    // 1-hour-digit
    leftOffset = leftOffset + elWidth + elHorizontalSpacing;
    m_theme->paint(p, QRectF(leftOffset, upperElementTop, elWidth, elHeight), 'e'+hours[1]+"-p1");
    m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight), 'e'+hours[1]+"-p2");
    
    // 10-minutes-digit
    leftOffset = leftOffset + elWidth + elHorizontalSpacing*4; // There's a gap between hours and minutes
    m_theme->paint(p, QRectF(leftOffset, upperElementTop, elWidth, elHeight), 'e'+minutes[0]+"-p1");
    m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight), 'e'+minutes[0]+"-p2");
    
    // 1-minute-digit
    leftOffset = leftOffset + elWidth + elHorizontalSpacing;
    m_theme->paint(p, QRectF(leftOffset, upperElementTop, elWidth, elHeight), 'e'+minutes[1]+"-p1");
    m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight), 'e'+minutes[1]+"-p2");

    // Make sure we don't get artifacts if an update gets called while animating
    if (m_animating) {
        Number oldMinutes = (QChar) minutes[1]; // 10-minutes digit
        --oldMinutes; // This is the digit which should be painted under the new one
        QString element;

//         leftOffset = leftOffset + elWidth + elHorizontalSpacing*4; // There's a gap between hours and minutes

        
        m_theme->paint(p, QRectF(leftOffset, upperElementTop, elWidth, elHeight), 'e'+minutes[1]+"-p1");
        element = QChar('e')+oldMinutes+QString("-p2");
        m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight), element);

        if (m_animationStep < 3 && m_animationStep > 0) {
            element = QChar('e')+oldMinutes+QString("-p1");
            m_theme->paint(p, QRectF(leftOffset, bottomElementTop-(elHeight/m_animationStep),
                                     elWidth, elHeight/m_animationStep), element);
        } else {
            m_theme->paint(p, QRectF(leftOffset, bottomElementTop, elWidth, elHeight/2), 'e'+minutes[1]+"-p2");
        }

        if (oldMinutes == '9') {
            Number oldMinutes = (QChar) minutes[0]; // 1-minutes digit
            --oldMinutes;
            QString element;

            m_theme->paint(p, QRectF(99, 20, elWidth, elHeight), 'e'+minutes[0]+"-p1");
            element = QString('e')+oldMinutes+QString("-p2");
            m_theme->paint(p, QRectF(99, 41, elWidth, elHeight), element);

            if (m_animationStep < 3 && m_animationStep > 0) {
                element = QString('e')+oldMinutes+QString("-p1");
                m_theme->paint(p, QRectF(99, 40-(elHeight/m_animationStep), elWidth, elHeight/m_animationStep), element);
            } else {
                m_theme->paint(p, QRectF(99, 41, elWidth, elHeight/2), 'e'+minutes[0]+"-p2");
            }

            if (oldMinutes == '9') { //NOTE: it's 9 instead of 5 because of how I defined operator--
                Number oldHours = (QChar) hours[1];
                --oldHours;
                QString element;

                m_theme->paint(p, QRectF(59, 20, elWidth, elHeight), 'e'+hours[1]+"-p1");
                element = QString('e')+oldHours+QString("-p2");
                m_theme->paint(p, QRectF(59, 41, elWidth, elHeight), element);

                if (m_animationStep < 3 && m_animationStep > 0) {
                    element = QString('e')+oldHours+QString("-p1");
                    m_theme->paint(p, QRectF(59, 40-(elHeight/m_animationStep), elWidth, elHeight/m_animationStep), element);
                } else {

                    m_theme->paint(p, QRectF(59, 41, elWidth, elHeight/2), 'e'+hours[1]+"-p2");
                }

                if (oldHours == '9') { //FIXME in case I'm displaying the AM/PM time
                    Number oldHours = (QChar) hours[0];
                    --oldHours;
                    QString element;

                    m_theme->paint(p, QRectF(39, 20, elWidth, elHeight), 'e'+hours[0]+"-p1");
                    element = QString('e')+oldHours+QString("-p2");
                    m_theme->paint(p, QRectF(39, 41, elWidth, elHeight), element);

                    if (m_animationStep < 3 && m_animationStep > 0) {
                        element = QString('e')+oldHours+QString("-p1");
                        m_theme->paint(p, QRectF(39, 40-(elHeight/m_animationStep), elWidth, elHeight/m_animationStep), element);
                    } else {

                        m_theme->paint(p, QRectF(39, 41, elWidth, elHeight/(6-m_animationStep)), 'e'+hours[0]+"-p2");
                    }
                } else {
                    m_theme->paint(p, QRectF(39, 20, elWidth, elHeight), 'e'+hours[0]+"-p1");
                    m_theme->paint(p, QRectF(39, 41, elWidth, elHeight), 'e'+hours[0]+"-p2");
                }
            }
        }
    }

    // Only show the date when there's enough room
    // We might want this configurable at some point.
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        // FIXME: this depends on the backgroundcolor of the theme, we'd want a matching contrast
        p->setPen(QPen(Qt::white));

        QString dateString = day + ' ' + month + ' ' + year;
        if (m_timezone != "Local")
        {
            dateString += "\n" + m_timezone;
        }
        p->drawText( QRectF(margin,
                            bottomElementTop+elHeight,
                            contentsRect.right()-margin,
                            contentsRect.bottom()) ,
                    dateString,
                    QTextOption(Qt::AlignHCenter)
                );
    }
}

#include "clock.moc"
