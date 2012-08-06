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

#include <QImage>
#include <QtCore/QTimer>

#include <KDebug>


class Notification::Private
{
public:
    Private()
        : timeout(0),
          urgency(0),
          hideTimer(0),
          expired(false),
          read(false)
    {
    }

    QString identifier;
    QString applicationName;
    QIcon applicationIcon;
    QString message;
    QString summary;
    int timeout;
    int urgency;
    QImage image;
    QTimer *deleteTimer;
    QTimer *hideTimer;
    bool expired;
    bool read;

    QHash<QString, QString> actions;
    QStringList actionOrder;
};

Notification::Notification(QObject *parent)
    : QObject(parent),
      d(new Private)
{
    d->deleteTimer = new QTimer(this);
    d->deleteTimer->setSingleShot(true);
    connect(d->deleteTimer, SIGNAL(timeout()), this, SLOT(destroy()));
}


Notification::~Notification()
{
    emit notificationDestroyed(this);
    delete d;
}

void Notification::destroy()
{
    emit notificationDestroyed(this);
    deleteLater();
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

QImage Notification::image() const
{
    return d->image;
}

void Notification::setImage(QImage image)
{
    d->image = image;
}

void Notification::setTimeout(int timeout)
{
    //show them at most 30 seconds
    if (!timeout) {
        d->timeout = 30 * 1000;
    } else {
        d->timeout = timeout;
    }

    if (d->urgency >= 2) {
        return;
    }

    if (!d->hideTimer) {
        d->hideTimer = new QTimer(this);
        d->hideTimer->setSingleShot(true);
        connect(d->hideTimer, SIGNAL(timeout()), this, SLOT(hide()));
    }
    d->hideTimer->start(d->timeout);
}

void Notification::setUrgency(int urgency)
{
    if (urgency != d->urgency) {
        d->urgency = urgency;
        if (urgency >= 2) {
            if (d->hideTimer) {
                d->hideTimer->stop();
            }
            d->deleteTimer->stop();
        } else {
            setTimeout(d->timeout);
        }
    }
}

int Notification::urgency() const
{
    return d->urgency;
}

QHash<QString, QString> Notification::actions() const
{
    return d->actions;
}


void Notification::setActions(const QHash<QString, QString> &actions)
{
    d->actions = actions;
    emit changed(this);
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

void Notification::remove()
{
    kDebug() << "remove requested but no handler implemented";
}

void Notification::linkActivated(const QString &link)
{
    Q_UNUSED(link)
    kDebug() << "link activation requested but no handler implemented";
}

void Notification::hide()
{
    d->expired = true;
    emit expired(this);
}

void Notification::startDeletionCountdown()
{
    if (d->urgency >= 2) {
        return;
    }

    //keep it available for 10 minutes
    d->deleteTimer->start(10*60*1000);
}

bool Notification::isExpired() const
{
    return d->expired;
}

void Notification::setRead(const bool read)
{
    d->read = read;
}

bool Notification::isRead() const
{
    return d->read;
}

void Notification::setDeleteTimeout(const int time)
{
    if (d->deleteTimer->interval() != time) {
        d->deleteTimer->start(time);
    }
}

int Notification::deleteTimeOut() const
{
    return d->deleteTimer->interval();
}



#include "notification.moc"
