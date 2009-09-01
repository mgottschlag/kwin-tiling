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
#include <KUrl>
#include <KLocalizedString>

namespace SystemTray
{


class Job::Private
{
public:
    Private() :
        state(Running),
        percentage(0),
        eta(0),
        timerId(0),
        killable(false),
        suspendable(false),
        shown(false)
    {
    }

    QString applicationName;
    QString applicationIconName;
    QString message;
    QString error;
    QString speed;
    QString destination;


    QMap<QString, qlonglong> totalAmounts;
    QMap<QString, qlonglong> processedAmounts;

    QList<QPair<QString, QString> > labels;

    State state;
    uint percentage;
    uint eta;
    int timerId;

    bool killable : 1;
    bool suspendable : 1;
    bool shown : 1;
};

Job::Job(QObject *parent)
    : QObject(parent),
      d(new Private)
{
    //delay a little the job to avoid the user to be distracted with short ones
    QTimer::singleShot(1500, this, SLOT(show()));
}

Job::~Job()
{
    delete d;
}

void Job::destroy()
{
    emit destroyed(this);
    deleteLater();
}

QString Job::applicationName() const
{
    return d->applicationName;
}

void Job::setApplicationName(const QString &applicationName)
{
    if (d->applicationName != applicationName) {
        d->applicationName = applicationName;
        scheduleChangedSignal();
    }
}

QString Job::applicationIconName() const
{
    return d->applicationIconName;
}

void Job::setApplicationIconName(const QString &applicationIcon)
{
    if (d->applicationIconName != applicationIcon) {
        d->applicationIconName = applicationIcon;
        scheduleChangedSignal();
    }
}

QString Job::message() const
{
    return d->message;
}

void Job::setMessage(const QString &message)
{
    if (d->message != message) {
        d->message = message;
        scheduleChangedSignal();
    }
}

QString Job::error() const
{
    return d->error;
}

void Job::setError(const QString &error)
{
    if (d->error != error) {
        d->error = error;
        scheduleChangedSignal();
    }
}

QString Job::speed() const
{
    return d->speed;
}

void Job::setSpeed(const QString &speed)
{
    if (d->speed != speed) {
        d->speed = speed;
        scheduleChangedSignal();
    }
}

QString Job::completedMessage() const
{
    KUrl location(d->destination);
    if (location.isValid()) {
        if (totalAmounts().value("files") > 1) {
            location.setFileName(QString());
        }

        QString destinationString;
        if (location.isLocalFile()) {
            destinationString = location.toLocalFile();
        } else {
            destinationString = location.prettyUrl();
        }

        QString destinationLink = QString("<a href=\"%1\">%1</a>").arg(destinationString);
        if (totalAmounts().value("files") > 1) {
            return i18np("%1 file, to: %2", "%1 files, to: %2", totalAmounts().value("files"),
                         destinationLink);
        } else {
            return destinationLink;
        }
    } else {
        return QString("%1: %2").arg(labels().value(0).first).arg(labels().value(0).second);
    }
}

ulong Job::eta() const
{
    return d->eta;
}

void Job::setEta(ulong eta)
{
    d->eta = eta;
}

QMap<QString, qlonglong> Job::totalAmounts() const
{
    return d->totalAmounts;
}

void Job::setTotalAmounts(QMap<QString, qlonglong> amounts)
{
    if (d->totalAmounts != amounts) {
        d->totalAmounts = amounts;
        scheduleChangedSignal();
    }
}

QMap<QString, qlonglong> Job::processedAmounts() const
{
    return d->processedAmounts;
}

void Job::setProcessedAmounts(QMap<QString, qlonglong> amounts)
{
    d->processedAmounts = amounts;
    scheduleChangedSignal();
}

Job::State Job::state() const
{
    return d->state;
}

void Job::setState(State state)
{
    if (d->state != state) {
        d->state = state;
        show();
        emit stateChanged(this);
    }
}

QList<QPair<QString, QString> > Job::labels() const
{
    return d->labels;
}

void Job::setLabels(QList<QPair<QString, QString> > labels)
{
    d->labels = labels;
    if (d->labels.count() > 1 && d->destination.isEmpty()) {
        d->destination = d->labels.value(1).second;
    }
    scheduleChangedSignal();
}

uint Job::percentage() const
{
    return d->percentage;
}

void Job::setPercentage(uint percentage)
{
    if (d->percentage != percentage) {
        d->percentage = percentage;
        scheduleChangedSignal();
    }
}

bool Job::isSuspendable() const
{
    return d->suspendable;
}

void Job::setSuspendable(bool suspendable)
{
    if (d->suspendable != suspendable) {
        d->suspendable = suspendable;
        scheduleChangedSignal();
    }
}

bool Job::isKillable() const
{
    return d->killable;
}

void Job::setKillable(bool killable)
{
    if (d->killable != killable) {
        d->killable = killable;
        scheduleChangedSignal();
    }
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

void Job::show()
{
    if (state() == Job::Running && !d->shown) {
        d->shown = true;
        emit ready(this);
    }
}

void Job::scheduleChangedSignal()
{
    if (d->shown && !d->timerId) {
        d->timerId = startTimer(0);
    }
}

void Job::timerEvent(QTimerEvent *)
{
    killTimer(d->timerId);
    d->timerId = 0;
    emit changed(this);
}

}

#include "job.moc"
