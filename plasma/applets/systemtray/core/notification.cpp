/***************************************************************************
 *   notification.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "notification.h"

#include <QtCore/QTimer>

#include <KDebug>


namespace SystemTray
{


class Notification::Private
{
public:
    Private()
        : timeout(0)
    {
    }

    QString applicationName;
    QIcon applicationIcon;
    QString message;
    QString summary;
    int timeout;

    QHash<QString, QString> actions;
    QStringList actionOrder;
};


Notification::Notification(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}


Notification::~Notification()
{
    emit destroyed(this);
    delete d;
}


QString Notification::applicationName() const
{
    return d->applicationName;
}


void Notification::setApplicationName(const QString &applicationName)
{
    d->applicationName = applicationName;
}


QIcon Notification::applicationIcon() const
{
    return d->applicationIcon;
}


void Notification::setApplicationIcon(const QIcon &applicationIcon)
{
    d->applicationIcon = applicationIcon;
}


QString Notification::message() const
{
    return d->message;
}


void Notification::setMessage(const QString &message)
{
    d->message = message;
}


QString Notification::summary() const
{
    return d->summary;
}


void Notification::setSummary(const QString &summary)
{
    d->summary = summary;
}


int Notification::timeout() const
{
    return d->timeout;
}


void Notification::setTimeout(int timeout)
{
    d->timeout = timeout;
    if (timeout) {
        QTimer::singleShot(timeout, this, SLOT(deleteLater()));
    }
}


QHash<QString, QString> Notification::actions() const
{
    return d->actions;
}


void Notification::setActions(const QHash<QString, QString> &actions)
{
    d->actions = actions;
}


QStringList Notification::actionOrder() const
{
    return d->actionOrder;
}


void Notification::setActionOrder(const QStringList &actionOrder)
{
    d->actionOrder = actionOrder;
}


void Notification::triggerAction(const QString &actionId)
{
    Q_UNUSED(actionId);
    kDebug() << "action triggered but no handler implemented";
}


}


#include "notification.moc"
