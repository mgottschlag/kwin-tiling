/***************************************************************************
 *   Copyright (C) 2008 Rob Scheepmaker <r.scheepmaker@student.utwente.nl> *
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

#include "extendertask.h"
#include <fixx11h.h>

#include <QtGui/QWidget> // QWIDGETSIZE_MAX

#include <plasma/popupapplet.h>
#include <plasma/widgets/iconwidget.h>


namespace SystemTray
{

class ExtenderTask::Private
{
public:
    Private(Plasma::PopupApplet *systemTray, Task *q)
        : q(q),
          iconWidget(0),
          systemTray(systemTray)
    {
    }

    Task *q;
    QString typeId;
    QString iconName;
    QIcon icon;
    Plasma::IconWidget *iconWidget;
    Plasma::PopupApplet *systemTray;
};


ExtenderTask::ExtenderTask(Plasma::PopupApplet *systemTray)
    : d(new Private(systemTray, this))
{
    setOrder(Last);
}


ExtenderTask::~ExtenderTask()
{
    emit taskDeleted(d->typeId);
    delete d;
}


bool ExtenderTask::isEmbeddable() const
{
    return true;
}

bool ExtenderTask::isValid() const
{
    return true;
}

QString ExtenderTask::name() const
{
    return i18n("Show or hide notifications and jobs");
}


QString ExtenderTask::typeId() const
{
    //FIXME: what should we return here?
    return "toggle_extender";
}


QIcon ExtenderTask::icon() const
{
    return d->icon;
}

void ExtenderTask::setIcon(const QString &icon)
{
    d->iconName = icon;
    if (d->iconWidget) {
        d->iconWidget->setIcon(icon);
    }
}

QGraphicsWidget* ExtenderTask::createWidget(Plasma::Applet *host)
{
    d->iconWidget = new Plasma::IconWidget(host);
    d->iconWidget->setToolTip(i18n("toggle visibility of notifications and jobs"));
    d->iconWidget->setIcon(d->iconName);
    d->iconWidget->setMinimumSize(22, 22);
    d->iconWidget->setMaximumSize(26, QWIDGETSIZE_MAX);
    connect(d->iconWidget, SIGNAL(clicked()), d->systemTray, SLOT(togglePopup()));
    return d->iconWidget;
}


}

#include "extendertask.moc"
