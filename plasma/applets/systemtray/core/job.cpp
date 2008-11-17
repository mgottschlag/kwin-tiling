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

#include "job.h"

#include <QtCore/QTimer>

#include <KDebug>

namespace SystemTray
{


class Job::Private
{
public:
    Private() :
        killable(false),
        suspendable(false),
        percentage(0)
    {
    }

    QString applicationName;
    QString applicationIconName;
    QString message;
    QString error;
    QString speed;

    QMap<QString, qlonglong> totalAmounts;
    QMap<QString, qlonglong> processedAmounts;

    QList<QPair<QString, QString> > labels;

    State state;

    bool killable;
    bool suspendable;

    uint percentage;
};

Job::Job(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

Job::~Job()
{
    emit destroyed(this);
    delete d;
}

QString Job::applicationName() const
{
    return d->applicationName;
}

void Job::setApplicationName(const QString &applicationName)
{
    d->applicationName = applicationName;
}

QString Job::applicationIconName() const
{
    return d->applicationIconName;
}

void Job::setApplicationIconName(const QString &applicationIcon)
{
    d->applicationIconName = applicationIcon;
}

QString Job::message() const
{
    return d->message;
}

void Job::setMessage(const QString &message)
{
    d->message = message;
}

QString Job::error() const
{
    return d->error;
}

void Job::setError(const QString &error)
{
    d->error = error;
}

QString Job::speed() const
{
    return d->speed;
}

void Job::setSpeed(const QString &speed)
{
    d->speed = speed;
}

QMap<QString, qlonglong> Job::totalAmounts() const
{
    return d->totalAmounts;
}

void Job::setTotalAmounts(QMap<QString, qlonglong> amounts)
{
    d->totalAmounts = amounts;
}

QMap<QString, qlonglong> Job::processedAmounts() const
{
    return d->processedAmounts;
}

void Job::setProcessedAmounts(QMap<QString, qlonglong> amounts)
{
    d->processedAmounts = amounts;
}

Job::State Job::state() const
{
    return d->state;
}

void Job::setState(State state)
{
    d->state = state;
}

QList<QPair<QString, QString> > Job::labels() const
{
    return d->labels;
}

void Job::setLabels(QList<QPair<QString, QString> > labels)
{
    d->labels = labels;
}

uint Job::percentage() const
{
    return d->percentage;
}

void Job::setPercentage(uint percentage)
{
    d->percentage = percentage;
}

bool Job::isSuspendable() const
{
    return d->suspendable;
}

void Job::setSuspendable(bool suspendable)
{
    d->suspendable = suspendable;
}

bool Job::isKillable() const
{
    return d->killable;
}

void Job::setKillable(bool killable)
{
    d->killable = killable;
}

void Job::suspend()
{
    kWarning() << "Suspend is not implemented in this job provider.";
}

void Job::resume()
{
    kWarning() << "Resume is not implemented in this job provider.";
}

void Job::stop()
{
    kWarning() << "Stop is not implemented in this job provider.";
}

}

#include "job.moc"
