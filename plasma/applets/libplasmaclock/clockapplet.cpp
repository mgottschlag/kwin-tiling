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

#include "clockapplet.h"

#include <math.h>

#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QSpinBox>
#include <QtCore/QTimeLine>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsView>
#include <QtCore/QDate>

#include <KColorScheme>
#include <KConfigDialog>
#include <KConfigGroup>
#include <KDatePicker>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>

#include <plasma/containment.h>
#include <plasma/corona.h>
#include <plasma/dataengine.h>
#include <plasma/dialog.h>
#include <plasma/extender.h>
#include <plasma/extenderitem.h>
#include <plasma/theme.h>

#include "ui_timezonesConfig.h"

class ClockApplet::Private
{
public:
    Private()
        : calendarDialog(0),
          calendar(0),
          view(0),
          timezone("Local")
    {}

    Ui::timezonesConfig ui;
    Plasma::Dialog *calendarDialog;
    KDatePicker *calendar;
    QGraphicsView *view;
    QString timezone;
    QPoint clicked;
    QStringList m_timeZones;
};

ClockApplet::ClockApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      d(new Private)
{
}

ClockApplet::~ClockApplet()
{
    delete d->calendar;
    delete d;
}

void ClockApplet::updateToolTipContent() {
    //QString timeString = KGlobal::locale()->formatTime(d->time, d->showSeconds);
    //TODO port to TOOLTIP manager
    /*Plasma::ToolTipData tipData;

    tipData.mainText = "";//d->time.toString(timeString);
    tipData.subText = "";//d->date.toString();
    //tipData.image = d->toolTipIcon;

    setToolTip(tipData);*/
}

void ClockApplet::createConfigurationInterface(KConfigDialog *parent)
{
    createClockConfigurationInterface(parent);

    QWidget *widget = new QWidget();
    d->ui.setupUi(widget);

    parent->addPage(widget, i18n("Time Zones"), icon());

    d->ui.localTimeZone->setChecked(isLocalTimezone());
    foreach(QString tz, d->m_timeZones) {
        d->ui.timeZones->setSelected(tz, true);
    }
    d->ui.timeZones->setEnabled(!isLocalTimezone());

    parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
    connect(parent, SIGNAL(applyClicked()), this, SLOT(configAccepted()));
    connect(parent, SIGNAL(okClicked()), this, SLOT(configAccepted()));

#if 0
#ifdef CLOCK_APPLET_CONF
    ui.localTimeZone->setChecked(isLocalTimezone());
    ui.timeZones->setEnabled(!isLocalTimezone());
    foreach (const QString &str, m_timeZones) {
        ui.timeZones->setSelected(str, true);
    }
#endif
#endif
}

void ClockApplet::createClockConfigurationInterface(KConfigDialog *parent)
{
    Q_UNUSED(parent)
}

void ClockApplet::clockConfigAccepted()
{

}

void ClockApplet::configAccepted()
{
    KConfigGroup cg = config();

    d->m_timeZones = d->ui.timeZones->selection();
    cg.writeEntry("timeZones", d->m_timeZones);

    QString newTimezone;

    if (d->ui.localTimeZone->isChecked() || d->m_timeZones.isEmpty()) {
        newTimezone = localTimezone();
    } else if (d->m_timeZones.contains(currentTimezone())) {
        newTimezone = currentTimezone();
    } else {
        newTimezone = d->m_timeZones.at(0);
    }

    changeEngineTimezone(currentTimezone(), newTimezone);

    setCurrentTimezone(newTimezone);
    cg.writeEntry("timezone", newTimezone);

    clockConfigAccepted();

    constraintsEvent(Plasma::SizeConstraint);
    update();
    emit configNeedsSaving();
}

void ClockApplet::adjustView()
{
    if (d->view && extender()) {
        d->view->setSceneRect(extender()->mapToScene(extender()->boundingRect()).boundingRect());
        d->view->resize(extender()->size().toSize());
    }
}

void ClockApplet::changeEngineTimezone(const QString &oldTimezone, const QString &newTimezone)
{
    Q_UNUSED(oldTimezone);
    Q_UNUSED(newTimezone);
}

void ClockApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton) {
        d->clicked = scenePos().toPoint();
        event->setAccepted(true);
        return;
    }

    Applet::mousePressEvent(event);
}

void ClockApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if ((d->clicked - scenePos().toPoint()).manhattanLength() < KGlobalSettings::dndEventDelay()) {
        showCalendar(event);
    }
}

void ClockApplet::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if(d->m_timeZones.count() <= 1 || isLocalTimezone())
        return;
    
    QString newTimezone;
    int current = d->m_timeZones.indexOf(currentTimezone());
    
    if(event->delta() > 0) {
        int previous = current - 1;
        if(previous < 0)
            newTimezone = d->m_timeZones.last();
        else
            newTimezone = d->m_timeZones.at(previous);
    }
    else {
        int next = current + 1;
        if(next > d->m_timeZones.count() - 1)
            newTimezone = d->m_timeZones.first();
        else
            newTimezone = d->m_timeZones.at(next);
    }
    
    changeEngineTimezone(currentTimezone(), newTimezone);
    setCurrentTimezone(newTimezone);
    
    // let's save our current timezone to be used per default
    KConfigGroup cg = config();
    cg.writeEntry("currentTimezone", newTimezone);
    emit configNeedsSaving();
    
    update();
}

void ClockApplet::initExtenderItem(Plasma::ExtenderItem *item)
{
    d->calendar = new KDatePicker;
    d->calendar->setMinimumSize(d->calendar->sizeHint());
    QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(d->calendar);
    item->setWidget(proxy);
    item->setTitle(i18n("Calendar"));
}

void ClockApplet::init()
{
    KConfigGroup cg = config();
    d->m_timeZones = cg.readEntry("timeZones", QStringList());
    
    Plasma::Extender *extender = new Plasma::Extender(this);
    containment()->corona()->addOffscreenWidget(extender);
    connect(extender, SIGNAL(geometryChanged()), this, SLOT(adjustView()));
}

void ClockApplet::showCalendar(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    adjustView();

    if (!d->calendarDialog) {
        if (!extender()) {
            // in case the subclass didn't call the parent init() properly
            ClockApplet::init();
        }

        if (!d->calendar) {
            Plasma::ExtenderItem *eItem = new Plasma::ExtenderItem(extender());
            eItem->setName("calendar");
            initExtenderItem(eItem);
        }

        d->calendarDialog = new Plasma::Dialog();
        d->calendarDialog->setWindowFlags(Qt::Popup);
        d->calendarDialog->setGraphicsWidget(extender());
    }

    if (d->calendarDialog->isVisible()) {
        d->calendarDialog->hide();
    } else {
        //kDebug();
        Plasma::DataEngine::Data data = dataEngine("time")->query(currentTimezone());
        d->calendar->setDate(data["Date"].toDate());
        d->calendarDialog->move(popupPosition(d->calendarDialog->sizeHint()));
        d->calendarDialog->show();
    }
}

void ClockApplet::setCurrentTimezone(const QString &tz)
{
    d->timezone = tz;
}

QString ClockApplet::currentTimezone() const
{
    return d->timezone;
}

bool ClockApplet::isLocalTimezone() const
{
    return d->timezone == localTimezone();
}

QString ClockApplet::localTimezone()
{
    return "Local";
}

#include "clockapplet.moc"
